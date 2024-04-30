#pragma once
#include <fishnet/AdjacencyContainer.hpp>
#include <fishnet/AdjacencyMap.hpp>
#include <unordered_map>
#include <expected>
#include <sstream>
#include <mgclient.hpp>
#include "MemgraphClient.hpp"



template<typename N>
concept DatabaseNode = requires(const N & node) {
    {node.key()} -> std::same_as<size_t>;
    {node.file()} -> std::same_as<const FileReference &>;
};

using namespace fishnet::graph;
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
    MemgraphClient client;
    AdjacencyMap<N,Hash,Equal> adjMap;
public:
    explicit MemgraphAdjacency(MemgraphClient && client):client(std::move(client)){}

    static std::expected<MemgraphAdjacency<N>,std::string> create(const mg::Client::Params & params ) {
        return MemgraphClient::create(params).transform([](auto && client){return MemgraphAdjacency<N>(std::move(client));});
    }

    static std::expected<MemgraphAdjacency<N>,std::string> create(std::string hostname, uint16_t port) {
        mg::Client::Params params;
        params.host = hostname;
        params.port = port;
        params.use_ssl = false;
        return create(params);
    }

    bool addAdjacency(N & from, N & to) noexcept {
        N copyFrom  = from;
        N copyTo = to;
        return addAdjacency(std::move(copyFrom),std::move(copyTo));
    }

    bool addAdjacency(N && from, N && to) noexcept {
        if (hasAdjacency(from,to))
            return false;
        if(client.insertEdge({from.key(),from.file()},{to.key(),to.file()})){
            adjMap.addAdjacency(std::move(from),std::move(to));
            return true;
        }
    }

    void addAdjacencies(util::forward_range_of<std::pair<N,N>> auto && edges) {

    }

    bool addNode(N & node) noexcept {
        if(contains(node))
            return false;
        N copy = node;
        return addNode(std::move(copy));
    }

    bool addNode(N && node) noexcept {
        if(contains(node)) {
            return false;
        }
        if(client.insertNode({node.key(),node.file()})){
            adjMap.addNode(std::move(node));
            return true;
        }
        return false;
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
        if(adjMap.contains(node))
            return true;
        return false;
        //TODO
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

    const MemgraphClient & getDatabaseConnection() const noexcept {
        return this->client;
    }



    void clear()  {
        this->keyToNodeMap.clear();
        //todo clear db but not all nodes
    }
};



