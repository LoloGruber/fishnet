#pragma once
#include "AbstractGraph.hpp"
#include "Edge.hpp"
#include <fishnet/AdjacencyContainer.hpp>
#include <fishnet/AdjacencyMap.hpp>
#include <concepts>

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
}