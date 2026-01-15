#pragma once
#include <vector>
#include <fishnet/Graph.hpp>
#include <fishnet/FunctionalConcepts.hpp>

namespace fishnet {

template<typename R>
struct ClusterResult {
    std::vector<std::vector<R>> clusters;
    std::vector<R> noise;
};

template<typename C, typename G>
concept ClusterAlgorithm = fishnet::graph::Graph<G> && fishnet::util::UnaryFunction<C, G, ClusterResult<typename G::node_type>>;

template<fishnet::graph::Graph G>
using ClusterAlgorithm_t = std::function<ClusterResult<typename G::node_type>(const G &)>;

} // namespace fishnet