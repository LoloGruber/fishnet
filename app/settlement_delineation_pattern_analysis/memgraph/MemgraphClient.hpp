#pragma once
#include <unordered_map>
#include <memory>
#include <expected>
#include <sstream>
#include <mgclient.hpp>
#include <filesystem>



namespace fishnet::graph::__impl{

class MemgraphClient{
private:
    std::unique_ptr<mg::Client> client;
    std::filesystem::path pathToFileOfVertices;
public:
    explicit MemgraphClient(std::unique_ptr<mg::Client> && clientPtr):client(std::move(clientPtr)){}

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

    bool insertEdge(int64_t from, int64_t to) noexcept {
        mg::Map params(2);
        std::stringstream query;
        params.Insert("from",mg::Value(from));
        params.Insert("to",mg::Value(to));
        query << "MERGE (f:Node {id:$from})" << std::endl;
        query << "MERGE (t:Node {id:$to})" << std::endl;
        query << "MERGE (f)-[:adj]->(t)" << std::endl;
        return client->Execute(query.str(),params.AsConstMap());
    }

    bool insertEdge(size_t from, size_t to) noexcept {
        return insertEdge(mg::Id::FromUint(from).AsInt(),mg::Id::FromUint(to).AsInt());
    }

    bool insertEdges(){

    }

    bool insertNode(int64_t node){

    }

    ~MemgraphClient(){
        client.reset(nullptr);
        mg::Client::Finalize();
    }
};
}


