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

    static inline NodeReference createNodeReference(const N & node) noexcept {
        return {node.key(),node.file()};
    }

    void storeInKeyNodeMap(const N & node) noexcept {
        keyToNodeMap.try_emplace(node.key(),node);
    }

    void storeInKeyNodeMap( N && node) noexcept {
        keyToNodeMap.try_emplace(node.key(),std::move(node));
    }

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
        if(client.insertEdge(createNodeReference(from),createNodeReference(to))){
            storeInKeyNodeMap(std::move(from));
            storeInKeyNodeMap(std::move(to));
            return true;
        }
        return false;
    }

    bool addAdjacencies(util::forward_range_of<std::pair<N,N>> auto && edges) {
        auto edgeReferences = std::views::all(edges) | std::views::transform([](const auto & pair){return std::make_pair(createNodeReference(pair.first),createNodeReference(pair.second));});
        if(client.insertEdges(edgeReferences)){
            for(auto && [from,to]:edges) {
                storeInKeyNodeMap(from);
                storeInKeyNodeMap(to);
            }
            return true;
        }
        return false;
    }

    bool addNode(N & node) noexcept {
        N copy = node;
        return addNode(std::move(copy));
    }

    bool addNode(N && node) noexcept {
        if(contains(node)) {
            return false;
        }
        if(client.insertNode(createNodeReference(node))){
            storeInKeyNodeMap(std::move(node));
            return true;
        }
        return false;
    }

    bool addNodes(util::forward_range_of<N> auto const& nodes) {
        if(client.insertNodes(std::views::all(nodes)
            | std::views::filter([this](const auto & node){return not keyToNodeMap.contains(node.key());})
            | std::views::transform([](const auto & node){return createNodeReference(node);})))
        {
            std::ranges::for_each(nodes,[this](const auto & node){storeInKeyNodeMap(node);});
            return true;
        }
        return false;

    }

    bool removeNode(const N & node)noexcept {
        if(client.removeNode(createNodeReference(node))){
            keyToNodeMap.erase(node.key());
            return true;
        }
        return false;
    }

    bool removeNodes(util::forward_range_of<N> auto const & nodes) {
        return false;
    }

    bool removeAdjacency(const N & from, const N & to) noexcept {
        return client.removeEdge(createNodeReference(from),createNodeReference(to));
    }

    bool removeAdjacencies(util::forward_range_of<std::pair<N,N>> auto const& edges){
        return false;
    }

    bool contains(const N & node) const noexcept {
        if(keyToNodeMap.contains(node.key()))
            return true;
        return client.containsNode(node.key());
    }

    bool hasAdjacency(const N & from, const N & to) const noexcept {
        return client.containsEdge(from.key(),to.key());
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
        //todo clear db but not all nodes
        this->keyToNodeMap.clear();
    }
};



