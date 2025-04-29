#pragma once
#include "SimpleGraph.hpp"
#include <fishnet/GraphModel.hpp>
#include <fishnet/DirectedAcyclicGraph.hpp>

namespace fishnet::graph {

class GraphFactory{
public:
    template<Node N, util::HashFunction<N> Hash = std::hash<N>,NodeBiPredicate<N> Equal = std::equal_to<N>>
    static auto UndirectedGraph(){
        return graph::__impl::SimpleGraph<UndirectedEdge<N,Hash,Equal>,AdjacencyMap<N,Hash,Equal>>();
    }

    template<Node N>
    static auto UndirectedGraph(AdjacencyContainer<N> auto && adjContainer){
        using AdjacencyContainer_t = std::remove_cvref_t<decltype(adjContainer)>;
        using Hash = typename AdjacencyContainer_t::hash_function;
        using Equal = typename AdjacencyContainer_t::equality_predicate;
        return graph::__impl::SimpleGraph<UndirectedEdge<N,Hash,Equal>,AdjacencyContainer_t>(std::move(adjContainer));
    }


    template<Node N, util::HashFunction<N> Hash = std::hash<N>,NodeBiPredicate<N> Equal = std::equal_to<N>>
    static auto DirectedGraph(){
        return graph::__impl::SimpleGraph<DirectedEdge<N,Hash,Equal>,AdjacencyMap<N,Hash,Equal>>();
    }

    template<Node N>
    static auto DirectedGraph(AdjacencyContainer<N> auto && adjContainer){
        using AdjacencyContainer_t = std::remove_cvref_t<decltype(adjContainer)>;
        using Hash = typename AdjacencyContainer_t::hash_function;
        using Equal = typename AdjacencyContainer_t::equality_predicate;
        return graph::__impl::SimpleGraph<DirectedEdge<N,Hash,Equal>,AdjacencyContainer_t>(std::move(adjContainer));
    }

    template<Node N>
    static auto DAG(AdjacencyContainer<N> auto && adjContainer){
        return DirectedAcyclicGraph(DirectedGraph<N>(std::move(adjContainer)));
    }

    template<Node N, util::HashFunction<N> Hash = std::hash<N>,NodeBiPredicate<N> Equal = std::equal_to<N>>
    static auto DAG(){
        return DirectedAcyclicGraph(DirectedGraph<N,Hash,Equal>());
    }
};
}