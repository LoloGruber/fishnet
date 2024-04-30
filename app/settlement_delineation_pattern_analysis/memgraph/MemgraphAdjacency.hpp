#pragma once
#include <fishnet/AdjacencyContainer.hpp>
#include <fishnet/AdjacencyMap.hpp>
#include <unordered_map>
#include <expected>
#include <sstream>
#include <mgclient.hpp>
#include "MemgraphClient.hpp"


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
class MemgraphAdjacency{
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
    std::unordered_map<size_t,N> keyToNodeMap;
    __impl::MemgraphClient client;
    AdjacencyMap<N,Hash,Equal> adjMap;
public:
    explicit MemgraphAdjacency(__impl::MemgraphClient && client):client(std::move(client)){}

    static std::expected<MemgraphAdjacency<N>,std::string> create(const mg::Client::Params & params ) {
        return __impl::MemgraphClient::create(params).transform([](auto && client){return MemgraphAdjacency<N>(std::move(client));});
    }

    static std::expected<MemgraphAdjacency<N>,std::string> create(std::string hostname, uint16_t port) {
        mg::Client::Params params;
        params.host = hostname;
        params.port = port;
        params.use_ssl = false;
        return create(params);
    }

    void addAdjacency(N & from, N & to) noexcept {
        N copyFrom  = from;
        N copyTo = to;
        addAdjacency(std::move(copyFrom),std::move(copyTo));
    }

    void addAdjacency(N && from, N && to) noexcept {
        if (hasAdjacency(from,to))
            return;
        if(client.insertEdge(from.key(),to.key()))
            adjMap.addAdjacency(std::move(from),std::move(to));
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
};
}


