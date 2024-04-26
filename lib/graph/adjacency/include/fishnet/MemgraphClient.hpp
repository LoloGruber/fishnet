#pragma once
#include "AdjacencyContainer.hpp"
#include <unordered_map>
#include <expected>
#include <sstream>
#include <mgclient.hpp>


namespace fishnet::graph{

template<typename N>
concept DatabaseNode = requires(const N & node) {
    {node.key()} -> std::same_as<size_t>;
};

/**
 * @brief Specialized Adjacency Container which connects to a (central) memgraph instance for graph model changes and queries
 * The memgraph database just stores the id of the nodes in the graph (obtained through node.key()), while a map tracks the mapping from key to the object
 * @tparam N type of node stored in adjacency container. 
 */
template<DatabaseNode N>
class MemgraphClient{
private:
    std::unordered_map<size_t,N> keyToNodeMap;
    // std::unique_ptr<mg::Client> client;
    // explicit MemgraphClient(std::unique_ptr<mg::Client> && clientPtr):client(std::move(clientPtr)){}
    MemgraphClient(){}
public:
    static std::expected<MemgraphClient<N>,std::string> create(const mg::Client::Params & params ) {
        auto clientPtr = mg::Client::Connect(params);
        if(not clientPtr){
            std::stringstream errorMessage;
            errorMessage << "Could not connect to memgraph database!" << std::endl;
            errorMessage << "\tHost: " <<params.host << std::endl;
            errorMessage << "\tPort: " << std::to_string(params.port) << std::endl;
            errorMessage << "\tUsername: " << params.username << std::endl;
            return std::unexpected(errorMessage.str());
        }
        return MemgraphClient();
    }

    static std::expected<MemgraphClient,std::string> create(std::string hostname, uint16_t port) {
        mg::Client::Params params;
        params.host = hostname;
        params.port = port;
        params.use_ssl = false;
        return create(params);
    }

    struct Equal{
        static bool operator()(const N & lhs, const N & rhs)  noexcept{
            return lhs.key() == rhs.key();
        }
    };

    struct Hash {
        static size_t operator()(const N & node)  noexcept {
            return node.key();
        }
    };
    using equality_predicate = Equal;
    using hash_function = Hash;

    void addAdjacency(N & from, N & to) noexcept {
        
    }

    ~MemgraphClient(){
        // client.reset(nullptr);
        mg::Client::Finalize();
    }



};

}