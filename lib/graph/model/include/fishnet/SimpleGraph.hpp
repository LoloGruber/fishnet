#pragma once
#include "NetworkConcepts.hpp"
#include "AbstractGraph.hpp"
#include <fishnet/AdjacencyContainer.hpp>
#include <fishnet/AdjacencyMap.hpp>
#include "Edge.hpp"

namespace fishnet::graph::__impl {
template<Edge E, AdjacencyContainer<typename E::node_type> AdjContainer>
class SimpleGraph: public AbstractGraph<SimpleGraph<E,AdjContainer>,E> {
private:
    AdjContainer adj = AdjContainer();
    using Base = AbstractGraph<SimpleGraph<E,AdjContainer>,E,AdjContainer>;
    using N = Base::node_type;

public:

    SimpleGraph():Base(){};
    SimpleGraph(util::forward_range_of<N> auto & nodes):Base(){
        addNodes(nodes);
    };

    SimpleGraph(util::forward_range_of<N> auto && nodes):Base(){
        addNodes(nodes);
    };

    bool addNode( N & node){
        return adj.addNode(node);
    }

    bool addNode(N && node){
        return adj.addNode(node);
    }

    template<typename... Args>
    bool addNode(N & node, Args... args){
        return Base::addNode(node,args...);
    }

    template<typename... Args>
    bool addNode(N && node, Args... args){
        return Base::addNode(node,args...);
    }

    bool addNodes(util::forward_range_of<N> auto & nodes){
        return Base::addNodes(nodes);
    }

    bool addNodes(util::forward_range_of<N> auto && nodes){
        return Base::addNodes(nodes);
    }

    bool containsNode(const N & node) const noexcept {
        return adj.contains(node);
    }

    void removeNode(const N & node) {
        adj.removeNode(node);
    }

    bool addEdge(N & from, N & to){
        if (not containsEdge(from,to)) {
            adj.addAdjacency(from,to);
            if constexpr(not E::isDirected()){
                adj.addAdjacency(to,from);  
            }
            return true;
        }
        return false;
    }

    bool addEdge(N && from, N && to){
        if (not containsEdge(from,to)){
            adj.addAdjacency(from,to);
            if constexpr(not E::isDirected()){
                adj.addAdjacency(to,from);
            }
            return true;
        }
        return false;
    }

    bool addEdge(const E & edge) {
        N from = edge.getFrom();
        N to = edge.getTo();
        return addEdge(from,to);
    }

    bool containsEdge(const N & from, const N & to) const noexcept {
        if constexpr(E::isDirected()) return adj.hasAdjacency(from,to);
        else return adj.hasAdjacency(from,to) && adj.hasAdjacency(to,from);
    }

    bool containsEdge(const E & edge) const noexcept {
        return containsEdge(edge.getFrom(),edge.getTo());
    }

    void removeEdge(const N & from, const N & to){
        adj.removeAdjacency(from,to);
        if constexpr(not E::isDirected()){
            adj.removeAdjacency(to,from);
        }
    }

    void removeEdge(const E & edge){
        removeEdge(edge.getFrom(),edge.getTo());
    }

    auto getNodes() const noexcept {
        return std::views::all(adj.nodes());
    }

    auto getEdges() const {
        std::unordered_set<E> edgeSet;
        for(const auto & [from,to]: adj.getAdjacencyPairs()){
            edgeSet.emplace(this->makeEdge(from,to));
        }
        std::vector<E> edges(edgeSet.begin(),edgeSet.end());
        return edges;
    }

    auto getNeighbours(const N & node) const noexcept {
        return adj.adjacency(node);
    }

    auto getReachableFrom(const N & node) const noexcept {
        if constexpr(E::isDirected()){
            auto isReachableFrom = [&](const N & from) {return containsEdge(from,node);};
            // return getNodes();
            return getNodes() | std::views::filter(isReachableFrom);
        }else {
            return getNeighbours(node);
        }        
    }

    auto getOutboundEdges(const N & node) const {
        std::vector<E> edges;
        if(not containsNode(node)) return edges;
        for(const auto & neighbour : getNeighbours(node)) {
            edges.emplace_back(this->makeEdge(node,neighbour));
        }
        return edges;
    }

    auto getInboundEdges(const N & node) const {
        std::vector<E> edges;
        if(not containsNode(node)) return edges;
        for(const auto & neighbour : getReachableFrom(node)) {
            edges.emplace_back(this->makeEdge(neighbour,node));
        }
        return edges;
    }

    void clear(){
        adj.clear();
    }

    ~SimpleGraph()=default;

};
}
