#pragma once
#include "Graph.hpp"

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
};
}