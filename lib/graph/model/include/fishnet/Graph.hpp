#pragma once
#include "AbstractGraph.hpp"
#include <concepts>
#include "Edge.hpp"
#include <fishnet/AdjacencyContainer.hpp>
#include <fishnet/AdjacencyMap.hpp>
#include "SimpleGraph.hpp"
#include "GraphDecorator.hpp"

namespace fishnet::graph {
/**
 * @brief Interface for graph types.
 * Every graph type has to inherit from AbstractGraph
 * @tparam GraphImpl graph implementation
 * @tparam E edge type
 * @tparam N node type
 */
template<typename GraphImpl, typename E=GraphImpl::edge_type, typename N = E::node_type, typename A = GraphImpl::adj_container_type>
concept Graph = Node<N> && Edge<E> && (std::derived_from<GraphImpl,AbstractGraph<GraphImpl,E,A>>) && requires(const GraphImpl & g, const N & n){
    {g.getNodes()} -> util::view_of<const N>;
    {g.getEdges()} -> util::forward_range_of<E>;
    {g.getNeighbours(n)} -> util::view_of<const N>;
    {g.getReachableFrom(n)} -> util::view_of<const N>;
    {g.getInboundEdges(n)} -> util::forward_range_of<E>;
    {g.getOutboundEdges(n)} -> util::forward_range_of<E>;
    typename GraphImpl::node_type;
    typename GraphImpl::edge_type;
    typename GraphImpl::adj_container_type;
};
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