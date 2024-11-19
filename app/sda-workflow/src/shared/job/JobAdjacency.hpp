#pragma once
#include <expected>
#include <sstream>
#include "Job.hpp"
#include <fishnet/AdjacencyContainer.hpp>
#include "MemgraphModel.hpp"
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
    MemgraphConnection dbConnection;
    friend JobDAGTest;
    JobAdjacency() = default;

    bool createConstraintsAndIndexes() const noexcept {
        return CipherQuery("CREATE CONSTRAINT ON (j:Job) ASSERT j.id IS UNIQUE").executeAndDiscard(dbConnection)
            && CipherQuery("CREATE CONSTRAINT ON (j:Job) ASSERT exists(j.file)").executeAndDiscard(dbConnection)
            && CipherQuery("CREATE CONSTRAINT ON (j:Job) ASSERT exists(j.state)").executeAndDiscard(dbConnection)
            && CipherQuery("CREATE CONSTRAINT ON (j:Job) ASSERT exists(j.type)").executeAndDiscard(dbConnection)
            && CipherQuery("CREATE INDEX ON :Job(id)").executeAndDiscard(dbConnection)
            && CipherQuery("CREATE EDGE INDEX ON :before").executeAndDiscard(dbConnection);
    }

    enum class QueryType{
        MERGE,MATCH
    };

    CipherQuery queryJob(const Job & job, QueryType queryType,std::string_view varName = "j") const noexcept {
        switch(queryType){
            case QueryType::MERGE:
            {
                std::stringstream attributes;
                attributes << "id:$"<<varName<<"id"
                << ",file:$"<<varName<<"file"
                << ",type:$"<<varName << "type"
                <<",state:$"<<varName<<"state";
                return CipherQuery()
                    .merge(Node(varName,Label::Job,attributes.str()))
                    .setInt(std::string(varName)+"id",job.id)
                    .set(std::string(varName)+"file",mg::Value(job.file.string()))
                    .set(std::string(varName)+"type",mg::Value(magic_enum::enum_name(job.type)))
                    .set(std::string(varName)+"state",mg::Value(magic_enum::enum_name(job.state)));
            }
            case QueryType::MATCH:
                return CipherQuery().match(Node(varName,Label::Job,"id:$"+std::string(varName)+"id"));
            default:
                return CipherQuery();
        }
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
    explicit JobAdjacency(MemgraphConnection && dbConnection):dbConnection(std::move(dbConnection)){
        if(not this->dbConnection.isConnected()) {
            throw std::runtime_error("Not connected to database");
        }
        if(not createConstraintsAndIndexes()) {
            throw std::runtime_error("Could not create constraints and indexes for \"Job\" label.");
        }
    }

    JobAdjacency(JobAdjacency && other):dbConnection(std::move(other.dbConnection)){}

    JobAdjacency & operator=(JobAdjacency && other) noexcept {
        this->dbConnection = std::move(other.dbConnection);
        return *this;
    }

    const MemgraphConnection & getConnection() const noexcept {
        return this->dbConnection;
    }

    bool addAdjacency(const Job & from, const Job & to)const noexcept {
       return queryJob(from,QueryType::MERGE,"f")
            .add(queryJob(to,QueryType::MERGE,"t"))
            .merge(Relation{.from=Node("f"),.label=Label::before,.to=Node("t")})
            .executeAndDiscard(dbConnection);
    }

    bool addAdjacencies(fishnet::util::forward_range_of<std::pair<Job,Job>> auto && jobRelationships)const noexcept {
        return std::ranges::fold_left(jobRelationships,true,[this](bool prev, auto && pair){
            auto && [f,t] = pair;
            return prev && addAdjacency(std::forward<Job>(f),std::forward<Job>(t));
        });
    }

    bool addNode(const Job & job) const noexcept {
        return queryJob(job,QueryType::MERGE).executeAndDiscard(dbConnection);
    }

    bool addNodes(fishnet::util::forward_range_of<Job> auto && jobs) const noexcept{
        return std::ranges::fold_left(jobs,true,[this](bool prev, auto && job){
            return prev && addNode(std::forward<Job>(job));
        });
    }

    bool updateJobState(const Job & job) const noexcept {
        return queryJob(job,QueryType::MATCH,"j").append("SET j.state=$state").set("state",mg::Value(magic_enum::enum_name(job.state))).executeAndDiscard(dbConnection);
    }

    bool removeNode(const Job & job) const noexcept{
        return queryJob(job,QueryType::MATCH,"j").append("DETACH DELETE j;").executeAndDiscard(dbConnection);
    }

    bool removeNodes(fishnet::util::forward_range_of<Job> auto && jobs) const noexcept {
        return std::ranges::fold_left(jobs,true,[this](bool prev, auto && job){
            return prev && removeNode(std::forward<Job>(job));
        });
    }

    bool removeAdjacency(const Job & from, const Job & to) const noexcept {
        return CipherQuery().match(Relation{
                .name="r",
                .from=Node("f",Label::Job,"id:$fid"),
                .label=Label::before,
                .to=Node("t",Label::Job,"id:$tid")
            })
            .setInt("fid",from.id)
            .setInt("tid",to.id)
            .del("r")
            .executeAndDiscard(dbConnection);
    }

    bool removeAdjacencies(fishnet::util::forward_range_of<std::pair<Job,Job>> auto && jobRelationships) const noexcept {
        return std::ranges::fold_left(jobRelationships,true,[this](bool prev, auto && pair){
            auto && [f,t] = pair;
            return removeAdjacency(f,t);
        });
    }

    bool clear() const noexcept {
        return CipherQuery().match(Node{.name="j",.label=Label::Job}).del("j").executeAndDiscard(dbConnection);
    }

    bool contains(const Job & job) const noexcept {
        CipherQuery q = queryJob(job,QueryType::MATCH,"j").ret("j.id");
        if(q.execute(dbConnection)){
            auto result = dbConnection->FetchAll();
            return result.has_value() && result->size() > 0;
        }   
        return false;
    }

    bool hasAdjacency(const Job & from, const Job & to) const noexcept {
        if(
            CipherQuery().match(Relation{
                .name="r",
                .from=Node("f",Label::Job,"id:$fid"),
                .label=Label::before,
                .to=Node("t",Label::Job,"id:$tid")
            })
            .setInt("fid",from.id)
            .setInt("tid",to.id)
            .ret("ID(r)")
            .execute(dbConnection)
        ){
            auto result = dbConnection->FetchAll();
            return result.has_value() && result->size() > 0;
        }   
        return false;
    }

    fishnet::util::forward_range_of<Job> auto adjacency(const Job & job) const noexcept {
        if(
            CipherQuery().match(Relation{
                .from=Node{.name="x",.label=Label::Job},
                .label=Label::before,
                .to=Node{.name="j",.label=Label::Job}
            }).ret("j").execute(dbConnection)
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
        if(CipherQuery().match(Node{.name="n",.label=Label::Job}).ret("n").execute(dbConnection)){
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
            CipherQuery().match(Relation{
                .from = Node{.name="f",.label=Label::Job},
                .label = Label::before,
                .to = Node{.name="t",.label=Label::Job}
            }).ret("f","t").execute(dbConnection)
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
