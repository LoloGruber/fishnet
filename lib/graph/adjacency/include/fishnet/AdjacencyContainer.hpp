#pragma once
#include <concepts>
#include <ranges>
#include <fishnet/UtilConcepts.hpp>
namespace fishnet::graph{
template<class C, typename N>
concept AdjacencyContainer= requires (C & container, const C & constContainer, N & nodeRef, const N & constNodeRef, N && nodeRval, bool b ){
    {C()};
    {container.addAdjacency(nodeRef,nodeRef)};
    {container.addAdjacency(nodeRval,nodeRval)};
    {container.addNode(nodeRef)} -> std::convertible_to<bool>;
    {container.addNode(nodeRval)} -> std::convertible_to<bool>;
    {container.removeNode(constNodeRef)};
    {container.removeAdjacency(constNodeRef,constNodeRef)};
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
