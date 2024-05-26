#pragma once
#include <concepts>
#include <ranges>
#include <fishnet/UtilConcepts.hpp>
namespace fishnet::graph{
/**
 * @brief Interface for an AdjacencyContainer
 * 
 * @tparam C container implementation type
 * @tparam N node type stored in the container
 */
template<class C, typename N>
concept AdjacencyContainer= requires (C & container, const C & constContainer, N & nodeRef, const N & constNodeRef, N && nodeRval, std::vector<N>  nodes, std::vector<std::pair<N,N>> adjacencies){
    {container.addAdjacency(constNodeRef,constNodeRef)};
    {container.addAdjacency(nodeRef,nodeRef)};
    {container.addAdjacency(nodeRval,nodeRval)};
    {container.addAdjacencies(adjacencies)};
    {container.addNode(nodeRef)} -> std::convertible_to<bool>;
    {container.addNode(nodeRval)} -> std::convertible_to<bool>;
    {container.addNode(constNodeRef)} -> std::convertible_to<bool>;
    {container.addNodes(nodes)} -> std::convertible_to<bool>;
    {container.removeNode(constNodeRef)};
    {container.removeNodes(nodes)};
    {container.removeAdjacency(constNodeRef,constNodeRef)};
    {container.removeAdjacencies(adjacencies)};
    {container.clear()};
    {constContainer.contains(constNodeRef)} -> std::convertible_to<bool>;
    {constContainer.hasAdjacency(constNodeRef,constNodeRef)} -> std::convertible_to<bool>;
    {constContainer.adjacency(constNodeRef)} -> util::view_of<const N>;
    {constContainer.nodes()} -> util::view_of<const N>;
    {constContainer.getAdjacencyPairs()} -> util::view_of<std::pair<const N,const N>>;
    typename C::hash_function;
    typename C::equality_predicate;
} ;
}
