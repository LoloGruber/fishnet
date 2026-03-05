#pragma once
#include <fishnet/Graph.hpp>
#include <fishnet/Concepts.hpp>

static auto completeGraph(std::ranges::forward_range auto const & nodes) {
    auto graph = fishnet::graph::UndirectedGraph<std::ranges::range_value_t<decltype(nodes)>>(nodes);
    for (auto && from : nodes) {
        for (auto && to : nodes) {
            if (from != to) {
                graph.addEdge(from, to);
            }
        }
    }
    return graph;
}
