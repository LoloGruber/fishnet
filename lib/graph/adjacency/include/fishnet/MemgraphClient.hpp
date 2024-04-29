#pragma once
#include "AdjacencyContainer.hpp"
#include <unordered_map>
#include <expected>
#include <sstream>
#include <mgclient.hpp>
#include "AdjacencyMap.hpp"


namespace fishnet::graph{

template<typename N>
concept DatabaseNode = requires(const N & node) {
    {node.key()} -> std::same_as<int64_t>;
};


/**
 * @brief Specialized Adjacency Container which connects to a (central) memgraph instance for graph model changes and queries
 * The memgraph database just stores the id of the nodes in the graph (obtained through node.key()), while a map tracks the mapping from key to the object
 * @tparam N type of node stored in adjacency container. 
 */
template<DatabaseNode N>
class MemgraphClient{
public: 
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
private:
    std::unordered_map<int64_t,N> keyToNodeMap;
    std::unique_ptr<mg::Client> client;
    AdjacencyMap<N,Hash,Equal> adjMap;
public:
    explicit MemgraphClient(std::unique_ptr<mg::Client> && clientPtr):client(std::move(clientPtr)){}

    static std::optional<MemgraphClient<N>> create(const mg::Client::Params & params ) {
        auto clientPtr = mg::Client::Connect(params);
        if(not clientPtr){
            std::cerr << "Could not connect to memgraph database!" << std::endl;
            std::cerr << "\tHost: " <<params.host << std::endl;
            std::cerr << "\tPort: " << std::to_string(params.port) << std::endl;
            std::cerr << "\tUsername: " << params.username << std::endl;
            return std::nullopt;
        }
        return std::optional<MemgraphClient<N>>(std::move(clientPtr));
    }

    static std::optional<MemgraphClient<N>> create(std::string hostname, uint16_t port) {
        mg::Client::Params params;
        params.host = hostname;
        params.port = port;
        params.use_ssl = false;
        return create(params);
    }



    void addAdjacency(N & from, N & to) noexcept {
        if (hasAdjacency(from,to))
            return;
        mg::Map params(2);
        std::stringstream query;
        params.Insert("from",mg::Value(from.key()));
        params.Insert("to",mg::Value(to.key()));
        query << "MERGE (f:Node {id:$from})" << std::endl;
        query << "MERGE (t:Node {id:$to})" << std::endl;
        query << "MERGE (f)-[:adj]->(t)" << std::endl;
        client->Execute(query.str(),params.AsConstMap());

    }

    void addAdjacency(N && from, N && to) noexcept {

    }

    void addAdjacencies(util::forward_range_of<std::pair<N,N>> auto && edges) {

    }

    bool addNode(N & node) noexcept {

    }

    bool addNode(N && node) noexcept {

    }

    bool addNodes(util::forward_range_of<N> auto && nodes) {

    }

    void removeNode(const N & node)noexcept {

    }

    bool removeNodes(util::forward_range_of<N> auto const & nodes) {
        
    }

    void removeAdjacency(const N & from, const N & to) noexcept {

    }

    void removeAdjacencies(util::forward_range_of<std::pair<N,N>> auto const& edges){

    }

    bool contains(const N & node) const noexcept {
        if(keyToNodeMap.contains(node.key()))
            return true;
        return false;
        
    }

    bool hasAdjacency(const N & from, const N & to) const noexcept {
        return false;
    }

    util::view_of<const N> auto adjacency(const N & node) const noexcept {
        return std::views::empty<const N>;
    }

    util::view_of<const N> auto nodes() const noexcept {
        return std::views::empty<const N>;
    }

    util::view_of<std::pair<const N, const N>> auto getAdjacencyPairs() const noexcept {
        return std::views::empty<std::pair<const N,const N>>;
    }



    void clear()  {
        this->keyToNodeMap.clear();
        //todo clear db but not all nodes
    }

    ~MemgraphClient(){
        client.reset(nullptr);
        mg::Client::Finalize();
    }
};
}


