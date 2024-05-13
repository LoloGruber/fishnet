#pragma once
#include "Edge.hpp"
#include "NetworkConcepts.hpp"
#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/AdjacencyContainer.hpp>
#include <fishnet/AdjacencyMap.hpp>
namespace fishnet::graph{
/**
 * @brief Abstract Graph Implementing using CRTP
 * 
 * @tparam GraphImpl graph implementation type
 * @tparam E edge type
 * @tparam AdjacencyMap<typename E::node_type,typename E::hash_function, typename E::equality_predicate> adjacency container type, default: AdjacencyMap
 */
template<class GraphImpl, Edge E, AdjacencyContainer<typename E::node_type> A = AdjacencyMap<typename E::node_type,typename E::hash_function, typename E::equality_predicate>>
class AbstractGraph {
private:
    GraphImpl & impl() noexcept {
        return static_cast<GraphImpl &>(*this);
    }

    GraphImpl const& impl() const noexcept {
        return static_cast<const GraphImpl & >(*this);
    }

    using N = E::node_type;

protected:
    AbstractGraph(){}

public:
    using node_type =  N;
    using edge_type =  E;
    using adj_container_type = A;


    bool addNode(N & node){
        return impl().addNode(node);
    }

    bool addNode(N && node){
        return impl().addNode(node);
    }

    bool addNodes(util::forward_range_of<N> auto & nodes){
        return std::ranges::all_of(nodes,[this](N & node){return addNode(node);});
    }

    bool addNodes(util::forward_range_of<N> auto && nodes){
        return std::ranges::all_of(nodes,[this](N && node){return addNode(node);});
    }

    template<typename... Args>
    bool addNode(N & node, Args... args){
        bool hasChanged = addNode(node);
        return hasChanged && addNode(args...);
    }

    template<typename... Args>
    bool addNode(N && node, Args... args){
        bool hasChanged = addNode(node);
        return hasChanged && addNode(args...);
    }

    bool containsNode(const N & node) const noexcept {
        return impl().containsNode(node);
    }

    void removeNode(const N & node) {
        impl().removeNode(node);
    }

    bool addEdge(N & from, N & to){
        return impl().addEdge(from,to);
    }

    bool addEdge(N&& from, N && to){
        return impl().addEdge(from,to);
    }

    bool addEdge(const E & edge){
        return impl().addEdge(edge);
    }

    void addEdges(util::forward_range_of<std::pair<N,N>> auto & edges){
        impl().addEdges(edges);
    }

    void addEdges(util::forward_range_of<E> auto const & edges) {
        std::vector<std::pair<N,N>> pairs;
        std::ranges::for_each(edges,[&pairs](const auto & e){pairs.emplace_back(e.getFrom(),e.getTo());});
        addEdges(pairs);
    }

    bool containsEdge(const N & from, const N & to) const noexcept {
        return impl().containsEdge(from,to);
    }

    bool containsEdge(const E & edge) const noexcept {
        return impl().containsEdge(edge);
    }

    void removeEdge(const N & from, const N & to){
        impl().removeEdge(from,to);
    }

    void removeEdge(const E & edge){
        impl().removeEdge(edge);
    }

    inline E makeEdge(const N & from, const N & to) const {
        return E(from,to);
    }

    auto getNodes() const noexcept{
        return impl().getNodes();
    }

    auto getEdges() const {
        return impl().getEdges();
    }

    auto getNeighbours(const N & node) const noexcept {
        return impl().getNeighbours(node);
    }

    auto getReachableFrom(const N & node) const noexcept {
        return impl().getReachableFrom(node);
    }

    auto getOutboundEdges(const N & node) const {
        return impl().getOutboundEdges(node);
    }

    auto getInboundEdges(const N & node) const {
        return impl().getInboundEdges(node);
    }

    const A & getAdjacencyContainer() const  {
        return impl().getAdjacencyContainer();
    }

    void clear(){
        impl().clear();
    }

    virtual ~AbstractGraph() = default;   
};
}