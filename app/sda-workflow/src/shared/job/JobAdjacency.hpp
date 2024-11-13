#pragma once
#include <expected>
#include <sstream>
#include "Job.hpp"
#include <fishnet/AdjacencyContainer.hpp>
#include "MemgraphConnection.hpp"
#include <magic_enum.hpp>
class JobDAGTest; // only required to access the private default constructor for testing

class JobAdjacency{
public:
    struct Hash{
        static size_t operator()(const Job & job) noexcept {
            return job.id;
        }
    };
    using equality_predicate = std::equal_to<Job>;
    using hash_function = Hash;
private:
    constexpr static std::string_view WORKFLOW_VARIABLE = "w";
    MemgraphConnection dbConnection;
    size_t workflowInstanceID;
    friend JobDAGTest;
    JobAdjacency() = default;

    bool createConstraintsAndIndexes() const noexcept {
        return Query("CREATE CONSTRAINT ON (w:Workflow) ASSERT w.id IS UNIQUE").executeAndDiscard(dbConnection)
            && Query("CREATE CONSTRAINT ON (j:Job) ASSERT exists(j.file)").executeAndDiscard(dbConnection)
            && Query("CREATE CONSTRAINT ON (j:Job) ASSERT exists(j.state)").executeAndDiscard(dbConnection)
            && Query("CREATE CONSTRAINT ON (j:Job) ASSERT exists(j.type)").executeAndDiscard(dbConnection)
            && Query("CREATE INDEX ON :Workflow(id)").executeAndDiscard(dbConnection)
            && Query("CREATE INDEX ON :Job(id)").executeAndDiscard(dbConnection)
            && Query("CREATE EDGE INDEX ON :before").executeAndDiscard(dbConnection);
    }

    bool registerWorkflow() const noexcept {
        return ParameterizedQuery()
            .append("MERGE (")
            .append(WORKFLOW_VARIABLE)
            .append(":Workflow{id:$wid})")
            .setInt("wid",this->workflowInstanceID)
            .executeAndDiscard(dbConnection);
    }

    enum class QueryType{
        MERGE,MATCH
    };

    ParameterizedQuery queryWorkflow() const noexcept {
        return ParameterizedQuery()
        .append("MATCH (")
        .append(WORKFLOW_VARIABLE)
        .append(":Workflow{id:$wid})")
        .setInt("wid",workflowInstanceID);
    }

    ParameterizedQuery queryJob(const Job & job, QueryType queryType,std::string_view varName = "j") const noexcept {
        std::stringstream builder;
        switch(queryType){
            case QueryType::MERGE:
                builder << magic_enum::enum_name(queryType) <<"(" << varName << ":Job {"
                << "id:$"<<varName<<"id"
                << ",file:$"<<varName<<"file"
                << ",type:$"<<varName << "type"
                <<",state:$"<<varName<<"state})";
                return ParameterizedQuery()
                    .line(builder.str())
                    .append("MERGE (")
                    .append(WORKFLOW_VARIABLE)
                    .append(")-[:contains]->(")
                    .append(varName)
                    .line(")")
                    .setInt(std::string(varName)+"id",job.id)
                    .set(std::string(varName)+"file",mg::Value(job.file.string()))
                    .set(std::string(varName)+"type",mg::Value(magic_enum::enum_name(job.type)))
                    .set(std::string(varName)+"state",mg::Value(magic_enum::enum_name(job.state)));
            case QueryType::MATCH:
                builder << magic_enum::enum_name(queryType) << "(" << varName << ":Job {id:$"<<varName<<"id})<-[:contains]-(w:Workflow{id:"<< workflowInstanceID<<"})";
                return ParameterizedQuery()
                    .line(builder.str())
                    .setInt(std::string(varName)+"id",job.id);
            default:
                return ParameterizedQuery();
        }
    }

    ParameterizedQuery queryAdjacency(const Job & from, std::string_view fromVariable, const Job & to, std::string_view toVariable) const noexcept {
        return (queryJob(from,QueryType::MATCH,fromVariable) + queryJob(to,QueryType::MATCH,toVariable));
    }

    static size_t asJobIdType(int64_t value) {
        return mg::Id::FromInt(value).AsUint();
    }

    Job fromQueryResult(mg::ConstNode const& resultNode) const noexcept {
        Job result;
        for(const auto & [key,val]:resultNode.properties()){
            if(key == "id")
                result.id = asJobIdType(val.ValueInt());
            if(key == "file")
                result.file = std::filesystem::path(val.ValueString());
            if(key == "type")
                result.type = magic_enum::enum_cast<JobType>(val.ValueString()).value_or(JobType::UNDEFINED);
            if(key == "state")
                result.state = magic_enum::enum_cast<JobState>(val.ValueString()).value_or(JobState::UNDEFINED);
        }
        return result;
    }

public:
    explicit JobAdjacency(MemgraphConnection && dbConnection, size_t workflowInstanceID):dbConnection(std::move(dbConnection)),workflowInstanceID(workflowInstanceID){
        if(not this->dbConnection.isConnected()) {
            throw std::runtime_error("Not connected to database");
        }
        if(not createConstraintsAndIndexes()) {
            throw std::runtime_error("Could not create constraints and indexes for \"Job\" label.");
        }
        if(not registerWorkflow()) {
            throw std::runtime_error("Could not register workflow with id "+workflowInstanceID);
        }
    }

    JobAdjacency(JobAdjacency && other):dbConnection(std::move(other.dbConnection)),workflowInstanceID(other.workflowInstanceID){}

    JobAdjacency & operator=(JobAdjacency && other) noexcept {
        this->dbConnection = std::move(other.dbConnection);
        this->workflowInstanceID = other.workflowInstanceID;
        return *this;
    }

    const MemgraphConnection & getConnection() const noexcept {
        return this->dbConnection;
    }

    bool addAdjacency(const Job & from, const Job & to)const noexcept {
       return (queryWorkflow() + queryJob(from,QueryType::MERGE,"f") + queryJob(to,QueryType::MERGE,"t")).
            line("MERGE (f)-[:before]->(t)")
            .executeAndDiscard(dbConnection);
    }

    bool addAdjacencies(fishnet::util::forward_range_of<std::pair<Job,Job>> auto && jobRelationships)const noexcept {
        return std::ranges::fold_left(jobRelationships,true,[this](bool prev, auto && pair){
            auto && [f,t] = pair;
            return prev && addAdjacency(std::forward<Job>(f),std::forward<Job>(t));
        });
    }

    bool addNode(const Job & job) const noexcept {
        return (queryWorkflow() + queryJob(job,QueryType::MERGE)).executeAndDiscard(dbConnection);
    }

    bool addNodes(fishnet::util::forward_range_of<Job> auto && jobs) const noexcept{
        return std::ranges::fold_left(jobs,true,[this](bool prev, auto && job){
            return prev && addNode(std::forward<Job>(job));
        });
    }

    bool updateJobState(const Job & job) const noexcept {
        return queryJob(job,QueryType::MATCH,"j").line("SET j.state=$state").set("state",mg::Value(magic_enum::enum_name(job.state))).executeAndDiscard(dbConnection);
    }

    bool removeNode(const Job & job) const noexcept{
        return queryJob(job,QueryType::MATCH,"j").line("DETACH DELETE j;").executeAndDiscard(dbConnection);
    }

    bool removeNodes(fishnet::util::forward_range_of<Job> auto && jobs) const noexcept {
        return std::ranges::fold_left(jobs,true,[this](bool prev, auto && job){
            return prev && removeNode(std::forward<Job>(job));
        });
    }

    bool removeAdjacency(const Job & from, const Job & to) const noexcept {
        return queryAdjacency(from,"f",to,"t")
            .line("MATCH (f)-[r:before]->(t)")
            .line("DETACH DELETE r")
            .executeAndDiscard(dbConnection);
    }

    bool removeAdjacencies(fishnet::util::forward_range_of<std::pair<Job,Job>> auto && jobRelationships) const noexcept {
        return std::ranges::fold_left(jobRelationships,true,[this](bool prev, auto && pair){
            auto && [f,t] = pair;
            return removeAdjacency(f,t);
        });
    }

    bool clear() const noexcept {
        return ParameterizedQuery("MATCH (j:Job)<-[:contains]-(w:Workflow {id:$wid})")
        .setInt("wid",workflowInstanceID)
        .line("DETACH DELETE j;")
        .executeAndDiscard(dbConnection);
    }

    bool contains(const Job & job) const noexcept {
        ParameterizedQuery q = queryJob(job,QueryType::MATCH,"j");
        if(q.line("RETURN j.id;").execute(dbConnection)){
            auto result = dbConnection->FetchAll();
            return result.has_value() && result->size() > 0;
        }   
        return false;
    }

    bool hasAdjacency(const Job & from, const Job & to) const noexcept {
        if(
            queryAdjacency(from,"f",to,"t")
            .line("MATCH (f)-[r:before]->(t)")
            .line("RETURN ID(r)")
            .execute(dbConnection)
        ){
            auto result = dbConnection->FetchAll();
            return result.has_value() && result->size() > 0;
        }   
        return false;
    }

    fishnet::util::forward_range_of<Job> auto adjacency(const Job & job) const noexcept {
        if(
            queryJob(job,QueryType::MATCH,"x")
            .line("MATCH (x)-[:before]->(j:Job)")
            .line("RETURN j")
            .execute(dbConnection)
        ){
            std::vector<Job> result;
            while(auto currentRow = dbConnection->FetchOne()){
                result.push_back(fromQueryResult(currentRow->at(0).ValueNode()));
            }
            return result;
        }
        std::cerr << "Could not execute query for \"adjacency(Job)\"" << std::endl;
        return std::vector<Job>();
    }

    fishnet::util::forward_range_of<Job> auto nodes() const noexcept {
        if(
            ParameterizedQuery("MATCH (j:Job)<-[:contains]-(:Workflow {id:$wid})")
            .setInt("wid",workflowInstanceID)
            .line("RETURN j")
            .execute(dbConnection)
        ){
            std::vector<Job> result;
            while(auto currentRow = dbConnection->FetchOne()){
                result.push_back(fromQueryResult(currentRow->at(0).ValueNode()));
            }
            return result;
        }
        std::cerr << "Could not execute query for \"nodes()\"" << std::endl;
        return std::vector<Job>();
    }

    fishnet::util::forward_range_of<std::pair<Job,Job>> auto getAdjacencyPairs() const noexcept {
        std::vector<std::pair<Job,Job>> result;
        if(
            ParameterizedQuery("MATCH (f:Job)<-[:contains]-(:Workflow {id:$wid})")
            .line("MATCH (f:Job)<-[:contains]-(:Workflow {id:$wid})")
            .setInt("wid",workflowInstanceID)
            .line("MATCH (f)-[:before]->(t)")
            .line("RETURN f,t")
            .execute(dbConnection)
        ){
            while(auto currentRow = dbConnection->FetchOne()){
                result.emplace_back(fromQueryResult(currentRow->at(0).ValueNode()),fromQueryResult(currentRow->at(1).ValueNode()));
            }
            return result;

        }
        std::cerr << "Could not execute query for \"getAdjacencyPairs()\"" << std::endl;
        return result;
    }
};
static_assert(fishnet::graph::AdjacencyContainer<JobAdjacency,Job>);