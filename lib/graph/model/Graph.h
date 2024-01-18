#pragma once
#include "AbstractGraph.h"
#include <concepts>
#include "Edge.h"
#include "AdjacencyContainer.h"
#include "AdjacencyMap.h"
#include "SimpleGraph.h"
#include "GraphDecorator.h"

namespace fishnet::graph {

template<typename GraphImpl, typename E=GraphImpl::edge_type, typename N = E::node_type>
concept Graph = Node<N> && Edge<E> && (std::derived_from<GraphImpl,AbstractGraph<GraphImpl,E>>) && requires(const GraphImpl & g, const N & n){
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



template<Node N, util::HashFunction<N> Hash = std::hash<N>, NodeBiPredicate<N> Equal = std::equal_to<N>, AdjacencyContainer<N> AdjContainer = AdjacencyMap<N,Hash,Equal>>
using UndirectedGraph = graph::__impl::SimpleGraph<UndirectedEdge<N,Hash,Equal>,AdjContainer>;

template<Node N, util::HashFunction<N> Hash = std::hash<N>, NodeBiPredicate<N> Equal = std::equal_to<N>, AdjacencyContainer<N> AdjContainer = AdjacencyMap<N,Hash,Equal>>
using DirectedGraph = graph::__impl::SimpleGraph<DirectedEdge<N,Hash,Equal>,AdjContainer>;

}