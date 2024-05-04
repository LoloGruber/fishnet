#pragma once
#include <unordered_map>
#include <memory>
#include <expected>
#include <sstream>
#include <mgclient.hpp>
#include <filesystem>
#include <fishnet/CollectionConcepts.hpp>

struct FileReference{
    int64_t fileId;
};

struct NodeReference{
    size_t nodeId;
    FileReference fileRef = FileReference(-1);
};


class MemgraphClient{
private:
    std::unique_ptr<mg::Client> mgClient;

    static int64_t asInt(size_t value) {
        return mg::Id::FromUint(value).AsInt();
    }

    static size_t asSizeT(int64_t value) {
        return mg::Id::FromInt(value).AsUint();
    }
public:
    class Query {
        protected:
            std::stringstream query;
        public:
            template<typename T>
            Query(T && value){
                append(std::forward<T>(value));
            }

            Query()=default;

            template<typename T>
            Query & append(T && value){
                query << std::forward<T>(value);
                return *this;
            }

            template<typename T>
            Query & line(T && value) {
                query << std::forward<T>(value) << std::endl;
                return *this;
            }

            Query & operator <<(auto && value) {
                return append(value);
            }

            std::stringstream & getQuery() {
                return query;
            }

            bool execute(const std::unique_ptr<mg::Client> & clientPtr) {
                return clientPtr->Execute(query.str());
            }

            bool executeAndDiscard(const std::unique_ptr<mg::Client> & clientPtr) {
                bool success = clientPtr->Execute(query.str());
                if(success)
                    clientPtr->DiscardAll();
                return success;
            }
    };


    class ParameterizedQuery{
        private:
            mg::Map params;
            std::stringstream query;
            static inline int defaultCapacity = 5;
        public:
            ParameterizedQuery(int paramCapacity):params(paramCapacity){}
            ParameterizedQuery():params(defaultCapacity){}

            // ParameterizedQuery(const ParameterizedQuery & other) {
            //     this->params = other.params;
            //     this->query = std::stringstream(other.query.str());
            // }

            template<typename T>
            ParameterizedQuery(T && value):params(defaultCapacity){
                this->append(std::forward<T>(value));
            }

            template<typename T>
            ParameterizedQuery(int capacity,T && value):params(capacity){
                append(std::forward<T>(value));
            }

            template<typename T>
            ParameterizedQuery & append(T && value){
                this->query << std::forward<T>(value);
                return *this;
            }

            template<typename T>
            ParameterizedQuery & line(T && value) {
                this->query << std::forward<T>(value) << std::endl;
                return *this;
            }

            ParameterizedQuery & operator <<(auto && value) {
                return append(value);
            }

            ParameterizedQuery & set(const std::string_view key, mg::Value && value) {
                params.Insert(key,value);
                return *this;
            }

            ParameterizedQuery & setInt(const std::string_view key, int64_t value){
                params.Insert(key,mg::Value(value));
                return *this;
            }

            ParameterizedQuery & setInt(const std::string_view key, size_t value){
                params.Insert(key,mg::Value(mg::Id::FromUint(value).AsInt()));
                return *this;
            }

            const mg::Map & getParameters() const noexcept {
                return params;
            }

            std::stringstream & getQuery() {
                return query;
            }

            ParameterizedQuery & debug() noexcept {
                std::cout << query.str() << std::endl;
                return *this;
            }

            bool execute(const std::unique_ptr<mg::Client> & clientPtr) {
                return clientPtr->Execute(query.str(),params.AsConstMap());
            }

            bool executeAndDiscard(const std::unique_ptr<mg::Client> & clientPtr) {
                bool success = clientPtr->Execute(query.str(),params.AsConstMap());
                if(success)
                    clientPtr->DiscardAll();
                return success;
            }
    };

    explicit MemgraphClient(std::unique_ptr<mg::Client> && clientPtr):mgClient(std::move(clientPtr)){
        if(not createConstraints()) {
            throw std::runtime_error("Could not create constraints. Check the database connection");
        }
    }

    MemgraphClient(MemgraphClient && other) {
        this->mgClient = std::move(other.mgClient);
    }

    static std::expected<MemgraphClient,std::string> create(const mg::Client::Params & params ) {
        auto clientPtr = mg::Client::Connect(params);
        if(not clientPtr){
            std::stringstream connectionError;
            connectionError << "Could not connect to memgraph database!" << std::endl;
            connectionError << "\tHost: " <<params.host << std::endl;
            connectionError << "\tPort: " << std::to_string(params.port) << std::endl;
            connectionError << "\tUsername: " << params.username << std::endl;
            return std::unexpected(connectionError.str());
        }
        return std::expected<MemgraphClient,std::string>(std::move(clientPtr));
    }

    static std::expected<MemgraphClient ,std::string> create(std::string hostname, uint16_t port) {
        mg::Client::Params params;
        params.host = hostname;
        params.port = port;
        params.use_ssl = false;
        return create(params);
    }

    bool createConstraints()const noexcept {
        return Query("CREATE CONSTRAINT ON (n:Node) ASSERT n.id IS UNIQUE").executeAndDiscard(mgClient)
            && Query("CREATE CONSTRAINT ON (f:File) ASSERT f.id IS UNIQUE").executeAndDiscard(mgClient)
            && Query("CREATE CONSTRAINT ON (f:File) ASSERT exists(f.path)").executeAndDiscard(mgClient);
    }


    std::optional<FileReference> addFileReference(const std::string & path) const noexcept{
        ParameterizedQuery query;
        query.append("MERGE (f:File {path:$path})");
        query.set("path",mg::Value(path));
        query.append("RETURN ID(f)");
        if(not query.execute(mgClient)) {
            return std::nullopt;
        }
        auto queryResult = mgClient->FetchAll();
        if(queryResult && queryResult->front().front().type() == mg::Value::Type::Int) {
            return FileReference(queryResult->front().front().ValueInt());
        }
        return std::nullopt;
    }

    bool insertEdge(NodeReference const & from, NodeReference const & to) const noexcept {
        return ParameterizedQuery(4)
            .line("MATCH (ff:File) WHERE ID(ff)=$fromFile")
            .setInt("fromFile",from.fileRef.fileId)
            .line("MATCH (ft:File) WHERE ID(ft)=$toFile")
            .setInt("toFile",to.fileRef.fileId)
            .line("MERGE (f:Node {id:$from})")
            .setInt("from",from.nodeId)
            .line("MERGE (t:Node {id:$to})")
            .setInt("to",to.nodeId)
            .line("MERGE (f)-[:adj]->(t)")
            .line("MERGE (f)-[:stored]->(ff)")
            .line("MERGE (t)-[:stored]->(ft)")
            .executeAndDiscard(mgClient);
    }

    bool insertEdge(size_t from, size_t to,FileReference const & fileRef) const noexcept {
        return insertEdge({from,fileRef},{to,fileRef});
    }

    bool insertEdges(util::forward_range_of<std::pair<NodeReference,NodeReference>> auto && edges)const noexcept{
        ParameterizedQuery query(1);
        query.line("UNWIND $data AS edge");
        query.line("MATCH (ff:File) WHERE ID(ff)=edge.fromFile");
        query.line("MATCH (tf:File) WHERE ID(tf)=edge.toFile");
        query.line("MERGE (f:Node {id:edge.from})");
        query.line("MERGE (t:Node {id:edge.to})");
        query.line("MERGE (f)-[:stored]->(ff)");
        query.line("MERGE (t)-[:stored]->(tf)");
        query.line("MERGE (f)-[:adj]->(t)");
        std::vector<mg::Value> data;
        for(auto && [from,to]:edges){
            mg::Map currentEdge{4};
            currentEdge.Insert("from",mg::Value(asInt(from.nodeId)));
            currentEdge.Insert("fromFile",mg::Value(asInt(from.fileRef.fileId)));
            currentEdge.Insert("to",mg::Value(asInt(to.nodeId)));
            currentEdge.Insert("toFile",mg::Value(asInt(to.fileRef.fileId)));
            data.push_back(mg::Value(std::move(currentEdge)));
        }
        query.set("data",mg::Value(mg::List(std::move(data))));
        return query.executeAndDiscard(mgClient);
    }

    bool insertNode(NodeReference const & node) const noexcept{
        return ParameterizedQuery(2)
            .line("MATCH (f:File) WHERE ID(f)=$fid")
            .setInt("fid",node.fileRef.fileId)
            .line("MERGE (n:Node {id:$nid})")
            .setInt("nid",node.nodeId)
            .line("MERGE (n)-[:stored]->(f)")
            .executeAndDiscard(mgClient);
    }

    bool insertNodes(util::forward_range_of<NodeReference> auto && nodes) const noexcept {
        ParameterizedQuery query(1);
        query.line("UNWIND $data AS node");
        query.line("MATCH (f:File) WHERE ID(f) = node.fileId");
        query.line("MERGE (n:Node {id:node.id})");
        query.line("MERGE (n)-[:stored]->(f)");
        std::vector<mg::Value> data;
        for(NodeReference const& node: nodes){
            mg::Map currentNode {2};
            currentNode.Insert("id",mg::Value(asInt(node.nodeId)));
            currentNode.Insert("fileId",mg::Value(asInt(node.fileRef.fileId)));
            data.push_back(mg::Value(std::move(currentNode)));
        }
        query.set("data",mg::Value(mg::List(std::move(data))));
        return query.executeAndDiscard(mgClient);
    }

    bool removeNode(NodeReference const & node) const noexcept {
        return ParameterizedQuery(1)
            .line("MATCH (n:Node {id:$id})")
            .setInt("id",node.nodeId)
            .line("DETACH DELETE n")
            .executeAndDiscard(mgClient);
    }

    bool removeNodes(util::forward_range_of<NodeReference> auto && nodes) const noexcept {
        ParameterizedQuery query(1);
        query.line("UNWIND $data as node");
        query.line("MATCH (n:Node {id:node.id})");
        query.line("DETACH DELETE n");
        std::vector<mg::Value> data;
        for(NodeReference const& node: nodes) {
            mg::Map currentNode {1};
            currentNode.Insert("id",mg::Value(asInt(node.nodeId)));
            data.push_back(mg::Value(std::move(currentNode)));
        }
        query.set("data",mg::Value(mg::List(std::move(data))));
        return query.executeAndDiscard(mgClient);
    }

    bool removeEdge(NodeReference const & from, NodeReference const & to) const noexcept {
        return ParameterizedQuery(2)
            .line("MATCH (f:Node {id:$fromId})-[a:adj]->(t:Node {id:$toId})")
            .setInt("fromId",from.nodeId)
            .setInt("toId",to.nodeId)
            .line("DETACH DELETE a")
            .executeAndDiscard(mgClient);
    }

    bool removeEdges(util::forward_range_of<std::pair<NodeReference,NodeReference>> auto && edges) const noexcept {
        ParameterizedQuery query(1);
        query.line("UNWIND $data as edge");
        query.line("MATCH (:Node {id:edge.from})-[a:adj]->(:Node {id:edge.to})");
        query.line("DETACH DELETE a");
        std::vector<mg::Value> data;
        for(const auto & [from,to]: edges) {
            mg::Map currentEdge {2};
            currentEdge.Insert("from",mg::Value(asInt(from.nodeId)));
            currentEdge.Insert("to",mg::Value(asInt(to.nodeId)));
            data.push_back(mg::Value(std::move(currentEdge)));
        }
        query.set("data",mg::Value(mg::List(std::move(data))));
        return query.executeAndDiscard(mgClient);
    }

    bool containsNode(size_t nodeId) const noexcept {
        if(ParameterizedQuery(1).line("MATCH (n:Node {id:$id})").setInt("id",nodeId).line("RETURN ID(n)").execute(mgClient)){
                auto result =  mgClient->FetchAll();
                return result.has_value() && result->size() > 0;
        }
        return false;
    }

    bool containsEdge(size_t from, size_t to) const noexcept {
        if(
            ParameterizedQuery(2)
            .line("MATCH (:Node {id:$fid})-[r:adj]->(:Node {id:$tid})")
            .line("RETURN ID(r)")
            .setInt("fid",from)
            .setInt("tid",to)
            .execute(mgClient)
        ){
            auto result = mgClient->FetchAll();
            return result.has_value() && result->size() > 0;
        }
        return false;
    }

    std::vector<size_t> adjacency(const NodeReference & node) const noexcept {
        if(
            ParameterizedQuery(1)
            .line("MATCH (:Node {id:$id})-[:adj]->(x:Node)")
            .setInt("id",node.nodeId)
            .line("RETURN x.id")
            .execute(mgClient)
        ){
            std::vector<size_t> output;
            while(auto currentRow = mgClient->FetchOne()){
                if(currentRow->front().type() == mg::Value::Type::Int){
                    size_t nodeId = asSizeT(currentRow->front().ValueInt());
                    output.push_back(nodeId);
                }
            }
            return output;
        }
        std::cerr << "Could not execute query for \"adjacency(Node)\"" << std::endl;
        return {};
    }

    std::unordered_map<size_t,std::vector<size_t>> edges() const noexcept {
        if(
            Query()
            .line("MATCH (f:Node)-[:adj]->(t:Node)")
            .line("RETURN f.id,t.id")
            .execute(mgClient)
        ){
            std::unordered_map<size_t,std::vector<size_t>> output;
            while(auto currentRow = mgClient->FetchOne()) {
                size_t from = asSizeT(currentRow->at(0).ValueInt());
                size_t to = asSizeT(currentRow->at(1).ValueInt());
                if(not output.contains(from))
                    output.try_emplace(from,std::vector<size_t>());
                output.at(from).push_back(to);
            }
            return output;
        }
        std::cerr << "Could not execute query for \"edges()\"" << std::endl;
        return {};
    }

    bool clearAll() const noexcept{
        return Query("MATCH (n)").line("DETACH DELETE n").executeAndDiscard(mgClient);
    }

    ~MemgraphClient(){
        mgClient.reset(nullptr);
        mg::Client::Finalize();
    }
};



