#pragma once
#include "GraphFactory.hpp"

namespace fishnet::graph{
/**
 * @brief Simple Undirected Graph type
 * 
 * @tparam N node type
 * @tparam Hash hasher on node type
 * @tparam Equal comparator on node type
 * @tparam AdjContainer Adjacency container used for the graph
 */
template<Node N, util::HashFunction<N> Hash = std::hash<N>, NodeBiPredicate<N> Equal = std::equal_to<N>, AdjacencyContainer<N> AdjContainer = AdjacencyMap<N,Hash,Equal>>
using UndirectedGraph = graph::__impl::SimpleGraph<UndirectedEdge<N,Hash,Equal>,AdjContainer>;

/**
 * @brief Simple Directed Graph type
 * 
 * @tparam N node type
 * @tparam Hash hasher on node type
 * @tparam Equal comparator on node type
 * @tparam AdjContainer Adjacency container used for the graph
 */
template<Node N, util::HashFunction<N> Hash = std::hash<N>, NodeBiPredicate<N> Equal = std::equal_to<N>, AdjacencyContainer<N> AdjContainer = AdjacencyMap<N,Hash,Equal>>
using DirectedGraph = graph::__impl::SimpleGraph<DirectedEdge<N,Hash,Equal>,AdjContainer>;
}