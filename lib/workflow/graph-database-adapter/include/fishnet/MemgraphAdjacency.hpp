#pragma once
#include <fishnet/AdjacencyContainer.hpp>
#include <unordered_map>
#include <unordered_set>
#include <expected>
#include <sstream>
#include <mgclient.hpp>
#include "MemgraphClient.hpp"

/**
 * @brief Concept for nodes to be stored in the memgraph database
 * Each node requires a key() (e.g. FishnetID) and a file reference to the file the node is stored in
 * @tparam N node type
 */
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
protected:
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

    void removeFromKeyNodeMap(const N & node) noexcept {
        keyToNodeMap.erase(node.key());
    }

public:
    explicit MemgraphAdjacency(MemgraphClient && client):client(std::move(client)){}

    bool addAdjacency(const N & from, const N & to) noexcept {
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

    bool addAdjacencies(fishnet::util::forward_range_of<std::pair<N,N>> auto && edges) {
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

    bool addNode(const N & node) noexcept {
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

    bool addNodes(fishnet::util::forward_range_of<N> auto && nodes) {
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

    bool removeNodes(fishnet::util::forward_range_of<N> auto && nodes) {
        if(client.removeNodes(
            std::views::all(nodes) | std::views::transform([](const auto & node){return createNodeReference(node);})
        )){
            std::ranges::for_each(nodes,[this](const auto & node){removeFromKeyNodeMap(node);});
            return true;
        }
        return false;
    }

    bool removeAdjacency(const N & from, const N & to) noexcept {
        return client.removeEdge(createNodeReference(from),createNodeReference(to));
    }

    bool removeAdjacencies(fishnet::util::forward_range_of<std::pair<N,N>> auto && edges){
        return client.removeEdges(
            std::views::all(edges) | std::views::transform([](const auto & pair){return std::make_pair(createNodeReference(pair.first),createNodeReference(pair.second));})
        );
    }

    bool contains(const N & node) const noexcept {
        if(keyToNodeMap.contains(node.key()))
            return true;
        return client.containsNode(node.key());
    }

    bool hasAdjacency(const N & from, const N & to) const noexcept {
        return client.containsEdge(from.key(),to.key());
    }

    fishnet::util::view_of<const N> auto adjacency(const N & node) const noexcept {
        std::vector<size_t> adjacentIds = client.adjacency(createNodeReference(node));
        return std::views::all(keyToNodeMap) 
            | std::views::filter([ids = std::move(adjacentIds)](const auto & keyValPair){return std::ranges::contains(ids,keyValPair.first);})
            | std::views::transform([](const auto & keyValPair){return keyValPair.second;});
    }

    fishnet::util::view_of<const N> auto nodes() const noexcept {
        return std::views::values(keyToNodeMap);
    }


    fishnet::util::view_of<std::pair<const N, const N>> auto getAdjacencyPairs() const noexcept {
        std::unordered_map<size_t,std::vector<size_t>> edgesMap = client.edges(); // adjacency map: node_id -> List<node_id>
        std::ranges::for_each(std::views::keys(keyToNodeMap),[&edgesMap](const size_t key){edgesMap.try_emplace(key);}); // add disconnected nodes
        return std::views::all(keyToNodeMap)
            | std::views::transform([edges=std::move(edgesMap),this](const auto & keyValPair){
                const auto & [key,node] = keyValPair;
                // view holding all neighbours in the adjacency list, which are part of this (sub)graph
                auto && neighboursPresentInKeyMap = std::views::filter(edges.at(key),[this](size_t neighbour){return this->keyToNodeMap.contains(neighbour);});
                return std::views::transform(neighboursPresentInKeyMap,[&node,this](const size_t neighbour){
                            return std::make_pair(node,this->keyToNodeMap.at(neighbour));
                });
            })
            | std::views::join; //flatten view
    }

    const MemgraphClient & getDatabaseConnection() const noexcept {
        return this->client;
    }

    void clear()  {
        this->client.removeNodes(std::views::keys(keyToNodeMap) | std::views::transform([](const size_t key){return NodeReference(key);}));
        this->keyToNodeMap.clear();
    }

    template<fishnet::util::forward_range_of<ComponentReference> ComponentRange = std::vector<ComponentReference>>
    bool loadNodes(fishnet::util::forward_range_of<N> auto && nodes, ComponentRange && componentIds = {}){
        std::unordered_set<NodeIdType> nodeIdSet;
        if(fishnet::util::isEmpty(componentIds)){
            auto allNodeIds = client.nodes();
            nodeIdSet.insert(std::ranges::begin(allNodeIds),std::ranges::end(allNodeIds));
        }else {
            auto nodeIds = client.nodesOfComponents(componentIds);
            nodeIdSet =  std::unordered_set<NodeIdType>(std::ranges::begin(nodeIds),std::ranges::end(nodeIds));
        }
        if(nodeIdSet.empty())
            return false;
        auto filteredViewOfNodes = nodes | std::views::filter([&nodeIdSet](const auto & node){
            return nodeIdSet.contains(node.key());
        });
        std::ranges::for_each(filteredViewOfNodes,[this](const auto & node){
            this->keyToNodeMap.try_emplace(node.key(),node);
        });
        return true;
    }
};



