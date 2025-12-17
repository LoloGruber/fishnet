#pragma once
#include <fishnet/Graph.hpp>
#include <fishnet/Concepts.hpp>

template<typename T>
static auto completeGraph(fishnet::util::range_of<T> auto const & nodes) {
    auto graph = fishnet::graph::UndirectedGraph<T>(nodes);
    for (auto && from : nodes) {
        for (auto && to : nodes) {
            if (from != to) {
                graph.addEdge(from, to);
            }
        }
    }
    return graph;
}