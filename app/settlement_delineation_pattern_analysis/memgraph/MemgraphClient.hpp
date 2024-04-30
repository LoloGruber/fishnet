#pragma once
#include <unordered_map>
#include <memory>
#include <expected>
#include <sstream>
#include <mgclient.hpp>
#include <filesystem>

struct FileReference{
    int64_t fileId;
};

struct NodeReference{
    size_t nodeId;
    FileReference const & fileRef;
};


class MemgraphClient{
private:
    std::unique_ptr<mg::Client> client;
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

    explicit MemgraphClient(std::unique_ptr<mg::Client> && clientPtr):client(std::move(clientPtr)){
        if(not createConstraints()) {
            throw std::runtime_error("Could not create constraints. Check the database connection");
        }
    }

    MemgraphClient(MemgraphClient && other) {
        this->client = std::move(other.client);
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

    bool createConstraints() noexcept {
        return Query("CREATE CONSTRAINT ON (n:Node) ASSERT n.id IS UNIQUE").executeAndDiscard(client)
            && Query("CREATE CONSTRAINT ON (f:File) ASSERT f.id IS UNIQUE").executeAndDiscard(client)
            && Query("CREATE CONSTRAINT ON (f:File) ASSERT exists(f.path)").executeAndDiscard(client);
    }


    std::optional<FileReference> addFileReference(const std::string & path) const{
        ParameterizedQuery query;
        query.append("MERGE (f:File {path:$path})");
        query.set("path",mg::Value(path));
        query.append("RETURN ID(f)");
        if(not query.execute(client)) {
            return std::nullopt;
        }
        auto queryResult = client->FetchAll();
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
            .executeAndDiscard(client);
    }

    bool insertEdge(size_t from, size_t to,FileReference const & fileRef) const noexcept {
        return insertEdge({from,fileRef},{to,fileRef});
    }

    bool insertEdges(){

    }

    bool insertNode(NodeReference const & node){
        return ParameterizedQuery(2)
            .line("MATCH (f:File) WHERE ID(f)=$fid")
            .setInt("fid",node.fileRef.fileId)
            .line("MERGE (n:Node {id:$nid})")
            .setInt("nid",node.nodeId)
            .line("MERGE (n)-[:stored]->(f)")
            .executeAndDiscard(client);
    }

    bool removeNode(NodeReference const & node) {
        return ParameterizedQuery(1)
            .line("MATCH (n:Node) {id:$id})")
            .setInt("id",node.nodeId)
            .line("DETACH DELETE n")
            .executeAndDiscard(client);
    }

    ~MemgraphClient(){
        client.reset(nullptr);
        mg::Client::Finalize();
    }
};



