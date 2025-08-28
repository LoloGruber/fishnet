#pragma once
#include <algorithm>
#include <fishnet/UtilConcepts.hpp>

#include "AdjacencyContainer.hpp"

namespace fishnet::graph{

/**
 * @brief A decorator for adjacency containers that adds additional functionality.
 *
 * @tparam AdjDelegate The type of the adjacency container to decorate.
 * @tparam N The type of the nodes in the graph.
 */
template<class AdjDelegate, typename N> requires AdjacencyContainer<AdjDelegate,N>
class AdjacencyContainerDecorator{
private:
    AdjDelegate delegate;
public:
    AdjacencyContainerDecorator(AdjDelegate delegate): delegate(std::move(delegate)) {}

    AdjacencyContainerDecorator(AdjDelegate && delegate) : delegate(std::move(delegate)) {}

    void addAdjacency(const N & from, const N & to) {
        delegate.addAdjacency(from, to);
    }

    void addAdjacency(N && from, N && to) {
        delegate.addAdjacency(std::move(from), std::move(to));
    }

    void addAdjacencies(util::forward_range_of<std::pair<N,N>> auto && pairs){
        delegate.addAdjacencies(std::forward<decltype(pairs)>(pairs));
    }

    bool addNode(const N & node){
        return delegate.addNode(node);
    }

    bool addNode(N && node){
        return delegate.addNode(std::move(node));
    }

    bool addNodes(util::forward_range_of<N> auto && nodes){
        return delegate.addNodes(std::forward<decltype(nodes)>(nodes));
    }

    void removeNode(const N & node){
        delegate.removeNode(node);
    }

    void removeNodes(util::forward_range_of<N> auto && nodes){
        delegate.removeNodes(std::forward<decltype(nodes)>(nodes));
    }

    void removeAdjacency(const N & from, const N & to){
        delegate.removeAdjacency(from,to);
    }

    void removeAdjacencies(util::forward_range_of<std::pair<N,N>> auto && pairs){
        delegate.removeAdjacencies(std::forward<decltype(pairs)>(pairs));
    }

    bool contains(const N & node) const noexcept {
        return delegate.contains(node);
    }

    bool hasAdjacency(const N & from, const N & to) const noexcept {
        return delegate.hasAdjacency(from,to);
    }

    auto adjacency(const N & node) const noexcept {
        return delegate.adjacency(node);
    }

    auto nodes() const noexcept {
        return delegate.nodes();
    }

    auto getAdjacencyPairs() const noexcept {
        return delegate.getAdjacencyPairs();
    }

    void clear() noexcept{
        delegate.clear();
    }

};
}