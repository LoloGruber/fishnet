#pragma once
#include <vector>
#include <algorithm>
#include <ranges>
#include <unordered_map>
#include <optional>
#include <type_traits>
#include <fishnet/Graph.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/Option.hpp>

namespace fishnet::graph::MST {

namespace __impl {

/**
 * @brief Union-Find (Disjoint Set Union) data structure for Kruskal's algorithm
 * 
 * @tparam N node type
 * @tparam Hash hash function for nodes
 * @tparam Equal equality predicate for nodes
 */
template<typename N, util::HashFunction<N> Hash, util::BiPredicate<N> Equal>
class UnionFind {
private:
    std::unordered_map<N, N, Hash, Equal> parent;
    std::unordered_map<N, size_t, Hash, Equal> rank;
    Equal eq = Equal();

public:
    UnionFind() = default;

    void makeSet(const N& node) {
        if (not parent.contains(node)) {
            parent.emplace(node, node);
            rank.emplace(node, 0);
        }
    }

    N find(const N& node) {
        auto it = parent.find(node);
        if (it == parent.end()) {
            makeSet(node);
            return node;
        }
        if (not eq(it->second, node)) {
            it->second = find(it->second);
        }
        return it->second;
    }

    bool unite(const N& a, const N& b) {
        N rootA = find(a);
        N rootB = find(b);
        if (eq(rootA, rootB)) {
            return false;
        }
        auto rankA = rank.at(rootA);
        auto rankB = rank.at(rootB);
        if (rankA < rankB) {
            std::swap(rootA, rootB);
            std::swap(rankA, rankB);
        }
        auto it = parent.find(rootB);
        it->second = rootA;
        if (rankA == rankB) {
            auto rankIt = rank.find(rootA);
            rankIt->second = rankA + 1;
        }
        return true;
    }

    bool connected(const N& a, const N& b) {
        return eq(find(a), find(b));
    }
};

/**
 * @brief Internal Kruskal's implementation
 * 
 * @tparam G graph type
 * @tparam W weight function type
 * @param graph input graph
 * @param weightFunction function computing edge weight from two nodes
 * @return fishnet::Option<G> containing the MST graph, or nullopt if disconnected
 */
template<Graph G, typename W>
requires std::totally_ordered<std::invoke_result_t<W, const typename G::node_type&, const typename G::node_type&>>
static fishnet::Option<G> kruskalImpl(
    const G& graph,
    W const& weightFunction
) {
    using N = typename G::node_type;
    using H = typename G::adj_container_type::hash_function;
    using Eq = typename G::adj_container_type::equality_predicate;
    using T = std::invoke_result_t<W, const N&, const N&>;

    auto nodes = graph.getNodes();
    size_t nodeCount = std::ranges::distance(nodes);

    if (nodeCount == 0) {
        return G();
    }

    if (nodeCount == 1) {
        G result;
        for (const auto& node : nodes) {
            result.addNode(node);
        }
        return result;
    }

    // Collect all edges with their weights
    struct WeightedEdgeRef {
        N from;
        N to;
        T weight;
    };

    std::vector<WeightedEdgeRef> weightedEdges;
    auto edges = graph.getEdges();
    for (const auto& edge : edges) {
        T weight = weightFunction(edge.getFrom(), edge.getTo());
        weightedEdges.push_back({edge.getFrom(), edge.getTo(), weight});
    }

    // Sort edges by weight ascending
    std::ranges::sort(weightedEdges, {}, &WeightedEdgeRef::weight);

    UnionFind<N, H, Eq> uf;
    for (const auto& node : nodes) {
        uf.makeSet(node);
    }

    G mst;
    for (const auto& node : nodes) {
        mst.addNode(node);
    }

    size_t edgesAdded = 0;
    size_t targetEdges = nodeCount - 1;

    for (const auto& we : weightedEdges) {
        if (not uf.connected(we.from, we.to)) {
            uf.unite(we.from, we.to);
            mst.addEdge(we.from, we.to);
            edgesAdded++;
            if (edgesAdded == targetEdges) {
                break;
            }
        }
    }

    // Check if graph is connected (MST has exactly n-1 edges)
    if (edgesAdded != targetEdges) {
        return std::nullopt;
    }

    return mst;
}

} // namespace __impl

/**
 * @brief Compute the Minimum Spanning Tree of a graph using Kruskal's algorithm.
 * 
 * Returns an Option containing the MST graph if the input graph is connected.
 * Returns nullopt if the graph is disconnected.
 * 
 * @tparam G graph type
 * @tparam W weight function type
 * @param graph input graph
 * @param weightFunction BiFunction computing the weight of an edge from its two endpoints
 * @return fishnet::Option<G> MST graph, or nullopt if graph is disconnected
 */
template<Graph G, typename W>
requires std::totally_ordered<std::invoke_result_t<W, const typename G::node_type&, const typename G::node_type&>>
fishnet::Option<G> kruskal(
    const G& graph,
    W const& weightFunction
) {
    return __impl::kruskalImpl<G, W>(graph, weightFunction);
}

} // namespace fishnet::graph::MST
