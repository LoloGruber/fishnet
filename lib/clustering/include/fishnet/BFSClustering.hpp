#pragma once
#include <fishnet/BFSAlgorithm.hpp>
#include "ClusterAlgorithm.hpp"

namespace fishnet {

template<typename T>
class BFSClustering {
private:
    fishnet::util::BiPredicate_t<T,T> inRelationPredicate;

public:
    BFSClustering(fishnet::util::BiPredicate<T,T> auto && inRelationPredicate)
        : inRelationPredicate(std::forward<fishnet::util::BiPredicate_t<T,T>>(inRelationPredicate)) {}

    BFSClustering(): inRelationPredicate(fishnet::util::TrueBiPredicate()) {}

    template<fishnet::graph::Graph G>
    ClusterResult<typename G::node_type> operator()(const G & graph) const 
    {
        static_assert(std::is_same_v<typename G::node_type, T>, "Graph node type must match clustering type T");
        ClusterResult<T> result;
        result.clusters = fishnet::graph::BFS::connectedComponents(graph, inRelationPredicate).get();   
        return result;
    }
};

static_assert(ClusterAlgorithm<BFSClustering<int>, fishnet::graph::UndirectedGraph<int>>);

} // namespace fishnet