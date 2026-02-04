#pragma once
#include <fishnet/GraphModel.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include "DirectedAcyclicGraph.hpp"
#include "WeightedGraph.hpp"
#include "SimpleGraph.hpp"

namespace fishnet::graph{
    
/**
 * @brief Simple Undirected Graph type
 * 
 * @tparam N node type
 * @tparam Hash hasher on node type
 * @tparam Equal comparator on node type
 * @tparam AdjContainer Adjacency container used for the graph
 */
template<Node N, util::HashFunction<N> Hash = std::hash<N>, util::BiPredicate<N> Equal = std::equal_to<N>, AdjacencyContainer<N> AdjContainer = AdjacencyMap<N,Hash,Equal>>
using UndirectedGraph = graph::__impl::SimpleGraph<UndirectedEdge<N,Hash,Equal>,AdjContainer>;

/**
 * @brief Simple Directed Graph type
 * 
 * @tparam N node type
 * @tparam Hash hasher on node type
 * @tparam Equal comparator on node type
 * @tparam AdjContainer Adjacency container used for the graph
 */
template<Node N, util::HashFunction<N> Hash = std::hash<N>, util::BiPredicate<N> Equal = std::equal_to<N>, AdjacencyContainer<N> AdjContainer = AdjacencyMap<N,Hash,Equal>>
using DirectedGraph = graph::__impl::SimpleGraph<DirectedEdge<N,Hash,Equal>,AdjContainer>;

class GraphFactory{
public:
    template<Node N, util::HashFunction<N> Hash = std::hash<N>,util::BiPredicate<N> Equal = std::equal_to<N>>
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


    template<Node N, util::HashFunction<N> Hash = std::hash<N>,util::BiPredicate<N> Equal = std::equal_to<N>>
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

    template<Node N, util::HashFunction<N> Hash = std::hash<N>,util::BiPredicate<N> Equal = std::equal_to<N>>
    static auto DAG(){
        return DirectedAcyclicGraph(DirectedGraph<N,Hash,Equal>());
    }
};
}