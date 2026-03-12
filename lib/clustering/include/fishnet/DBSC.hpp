#include <fishnet/Graph.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/Statistics.hpp>
#include <fishnet/Constants.hpp>
#include <ranges>
#include <unordered_map>
#include <queue>
#include <cmath>
#include <cassert>
#include "ClusterAlgorithm.hpp"


namespace fishnet {

template<typename T>
class DBSC {
private:
    const double eps; // TODO compute automatically from the data and base reachability on attribute difference and spatial distance 
    double T1 = NAN; // attribute difference threshold for spatial reachability, will be computed from the data
    const int beta;
    const int minPts = 2;
    fishnet::util::BiFunction_t<T,T,double> distanceFunction;
    fishnet::util::UnaryFunction_t<T, double> attributeExtractor;
    std::unordered_map<size_t, bool> visitedNodes; // node id to visited flag

    struct ClusterNode {
        T node;
        size_t id;
        double attributeValue;
        mutable double densityIndicator = 0.0;
        mutable double meanAttributeDiff = 0.0; // mean attribute difference to neighbours
        mutable double nearestAttributeDiff = 0.0; // attribute difference to nearest neighbour

        bool operator==(const ClusterNode & other) const {
            return node == other.node;
        }
    };

    struct ClusterNodeHash {
        size_t operator()(const ClusterNode & node) const {
            return node.id;
        }
    };

    struct ClusterNodeOrdering {
        bool operator()(const ClusterNode & lhs, const ClusterNode & rhs) const {
            if (! fishnet::math::areEqual(lhs.densityIndicator, rhs.densityIndicator)) {
                return lhs.densityIndicator > rhs.densityIndicator;
            }
            return lhs.meanAttributeDiff < rhs.meanAttributeDiff;
        }
    };

    double distance(const T & lhs, const T & rhs) const noexcept {
        return distanceFunction(lhs, rhs);
    }

    double inline distance(const ClusterNode & lhs, const ClusterNode & rhs) const noexcept {
        return distanceFunction(lhs.node, rhs.node);
    }

    bool inline spatially_directly_reachable(const ClusterNode & p, const ClusterNode & q)const noexcept {
        return distance(p, q) <= eps;
    }

    constexpr auto getClusterGraphType(fishnet::graph::Graph auto const & graph) {
        if constexpr (std::remove_cvref_t<decltype(graph)>::edge_type::isDirected()) {
            return fishnet::graph::DirectedGraph<ClusterNode, ClusterNodeHash>();
        }
        else {
            return fishnet::graph::UndirectedGraph<ClusterNode, ClusterNodeHash>();
        }
    }

    auto getClusterGraph(fishnet::graph::Graph auto const & graph) {
        auto clusterGraph = getClusterGraphType(graph);
        size_t nodeIndex = 0;
        std::unordered_map<T, ClusterNode> nodeMap;
        for(const auto & node : graph.getNodes()){
            nodeMap.try_emplace(node, ClusterNode{
                .node = node,
                .id = nodeIndex++,
                .attributeValue = attributeExtractor(node)
            });
        }
        for(const auto & node: graph.getNodes()){
            for(const auto & nbr: graph.getNeighbours(node)){
                clusterGraph.addEdge(nodeMap[node], nodeMap[nbr]);
            }
        }
        return clusterGraph;
    }

    double prepareClusteringData(fishnet::graph::Graph auto & graph) {
        /* Update cluster nodes in the trimmed graph with density indicators and mean attribute difference and nearest neighbour attribute difference */
        auto nodes = graph.getNodes();
        for(const ClusterNode & clusterNode : nodes){
            visitedNodes[clusterNode.id] = false;
            auto neighbors = graph.getNeighbours(clusterNode);
            double N_size = static_cast<double>(fishnet::util::size(neighbors));
            if(N_size == 0) continue;
            double N_SDR = static_cast<double>(std::ranges::count_if(neighbors, [&](const auto & nbr) {
                return spatially_directly_reachable(clusterNode, nbr);
            }));
            double meanAttributeDiff = fishnet::math::mean(neighbors, [&](const auto & nbr) {
                return std::abs(clusterNode.attributeValue - nbr.attributeValue);
            }).value_or(0.0);

            const ClusterNode & nearestNeighbour = *std::ranges::min_element(neighbors, [&](const auto & lhs, const auto & rhs) {
                return distance(clusterNode, lhs) < distance(clusterNode, rhs);
            });
            clusterNode.nearestAttributeDiff = std::abs(clusterNode.attributeValue - nearestNeighbour.attributeValue);
            clusterNode.meanAttributeDiff = meanAttributeDiff;
            clusterNode.densityIndicator = N_SDR + N_SDR/N_size;
        }
        double avgAttributeDiff = fishnet::math::mean(nodes,&ClusterNode::nearestAttributeDiff).value_or_throw("Illegal state, graph has at least one node, so attributeDiffs should not be empty");
        double stdAttributeDiff = fishnet::math::std(nodes, &ClusterNode::nearestAttributeDiff).value_or_throw("Illegal state, graph has at least one node, so attributeDiffs should not be empty"); 
        const double T1 = fishnet::math::mean(nodes | std::views::filter([avgAttributeDiff, stdAttributeDiff](const ClusterNode & data) {
            return avgAttributeDiff - 3 * stdAttributeDiff <= data.nearestAttributeDiff && data.nearestAttributeDiff <= avgAttributeDiff + 3 * stdAttributeDiff;
        }), &ClusterNode::nearestAttributeDiff).value_or_throw("Illegal state, there should be at least one non-outlier attribute difference, so the filtered range should not be empty");
        return T1;
    }

    size_t count_spatially_directly_reachable(const ClusterNode & node, fishnet::graph::Graph auto const & graph) const noexcept {
        return std::ranges::count_if(graph.getNeighbours(node), [this,&node](const auto & nbr) {
            return spatially_directly_reachable(node, nbr);
        });
    } 

    bool visited(const ClusterNode & node) const {
        return visitedNodes.at(node.id);
    }

    void markVisited(const ClusterNode & node) {
        visitedNodes[node.id] = true;
    }

    bool reachable(const ClusterNode & core, const ClusterNode & q, fishnet::util::forward_range_of<ClusterNode> auto && temporalCluster) {
        double avg_clu = core.attributeValue;
        for(const auto & node : temporalCluster){
            avg_clu += node.attributeValue;
        }
        avg_clu = avg_clu/(1 + static_cast<double>(fishnet::util::size(temporalCluster)));
        return std::abs(q.attributeValue - avg_clu) <= T1;
    }

    std::vector<ClusterNode> expand(const ClusterNode & core, fishnet::graph::Graph auto & graph) {
        std::vector<ClusterNode> cluster;
        cluster.push_back(core);

        // (i)+(ii): Pre-pass over direct (order-1) neighbors to establish the initial cluster.
        // This bootstraps avg(CLU) with a meaningful local mean before any more distant
        // node is evaluated — the reachable() predicate is only meaningful once this
        // initial cluster is populated.
        std::vector<ClusterNode> directNeighbours;
        for (const ClusterNode & nbr : graph.getNeighbours(core)) {
            if (!visited(nbr) && count_spatially_directly_reachable(nbr, graph) > 0)
                directNeighbours.push_back(nbr);
        }
        std::ranges::sort(directNeighbours, ClusterNodeOrdering{});
        for (const ClusterNode & nbr : directNeighbours) {
            if (spatially_directly_reachable(core, nbr) && reachable(core, nbr, cluster)) {
                markVisited(nbr);
                cluster.push_back(nbr);
            }
        }

        // (iii): Expand further using the full beta-order neighborhood.
        // The queue is seeded with all hops 1..beta sorted by descending density;
        // already-visited nodes (admitted in step ii) are skipped immediately,
        // so effectively only hops 2..beta are processed here.
        // Newly admitted nodes are enqueued so expansion continues iteratively outward.
        auto betaOrderNeighbours = fishnet::util::toVector(
            fishnet::graph::BFS::neighborhood(graph, core, beta).getNodes());
        std::ranges::sort(betaOrderNeighbours, ClusterNodeOrdering{});

        std::queue<ClusterNode> seedQueue;
        for (const ClusterNode & nbr : betaOrderNeighbours) {
            if(!visited(nbr) && count_spatially_directly_reachable(nbr, graph) > 0)
                seedQueue.push(nbr);
        }
        while (!seedQueue.empty()) {
            const ClusterNode seed = seedQueue.front();
            seedQueue.pop();
            if (visited(seed) || count_spatially_directly_reachable(seed, graph) == 0)
                continue;
            if (spatially_directly_reachable(core, seed) && reachable(core, seed, cluster)) {
                markVisited(seed);
                cluster.push_back(seed);
                for (const ClusterNode & nbr : graph.getNeighbours(seed)) {
                    if (!visited(nbr) && count_spatially_directly_reachable(nbr, graph) > 0)
                        seedQueue.push(nbr);
                }
            }
        }
        return cluster;
    }

public: 
    DBSC(double eps, int beta, fishnet::util::BiFunction<T,T,double> auto && distanceFunction, fishnet::util::UnaryFunction<T, double> auto && attributeExtractor) 
        : eps(eps), beta(beta), distanceFunction(std::forward<fishnet::util::BiFunction_t<T,T,double>>(distanceFunction)), attributeExtractor(std::forward<fishnet::util::UnaryFunction_t<T, double>>(attributeExtractor)) {}

    auto cluster(fishnet::graph::Graph auto const & input_graph) {
        static_assert(std::derived_from<typename std::decay_t<decltype(input_graph)>::node_type, T>, "Graph node type must be compatible with DBSC point type");
        auto graph = getClusterGraph(input_graph);
        auto initalEdges = graph.getEdges();
        auto edgeDistanceMapper = [this](const auto & edge) {return distance(edge.getFrom(), edge.getTo());};
        const double global_mean = fishnet::math::mean(initalEdges, edgeDistanceMapper).value_or(0.0);
        const double global_variation = fishnet::math::std(initalEdges, edgeDistanceMapper, global_mean).value_or(0.0);
        auto local_mean = [&](const ClusterNode & node) -> double{
            return fishnet::math::mean(graph.getNeighbours(node), [this,&node](const auto & nbr) {
                return distance(node, nbr);
            }).value_or(0.0);
        };
        auto local_variation = [&](const ClusterNode & node) -> double{
            return fishnet::math::std(graph.getNeighbours(node), [this,&node](const auto & nbr) {
                return distance(node, nbr);
            }).value_or(0.0);
        };

        /* Remove global long edges */
        for(const auto & node : graph.getNodes()){
            double global_distance_constraint = global_mean + (global_mean/local_mean(node)) * global_variation;
            for(const auto & nbr : graph.getNeighbours(node)){
                if(distance(node, nbr) > global_distance_constraint){
                    graph.removeEdge(node, nbr);
                }
            }
        }

        /* Remove local long edges*/
        for (const auto & node: graph.getNodes()){
            auto order2_graph = fishnet::graph::BFS::neighborhood(graph, node, 2);
            auto order2_mean = fishnet::math::mean(order2_graph.getEdges(), edgeDistanceMapper).value_or(0.0);
            auto mean_local_variation = fishnet::math::mean(order2_graph.getNodes(), local_variation).value_or(0.0);
            auto local_distance_constraint = order2_mean + beta * mean_local_variation;
            for(const auto & edge: order2_graph.getEdges()){
                if(distance(edge.getFrom(), edge.getTo()) > local_distance_constraint){
                    graph.removeEdge(edge);
                }
            }
        }
        /* Prepare clustering data */
        this->T1 = prepareClusteringData(graph);
        std::vector<ClusterNode> clusteringData(graph.getNodes().begin(), graph.getNodes().end());
        std::ranges::sort(clusteringData, ClusterNodeOrdering{});
        assert(T1 != NAN);
        /* Clustering */
        ClusterResult<T> clusterResult;
        for(const ClusterNode & core : clusteringData){
            if(visited(core)) 
                continue;
            markVisited(core);
            auto result = expand(core, graph);
            if (fishnet::util::size(result) >= minPts) {
                clusterResult.clusters.push_back(fishnet::util::toVector(result | std::views::transform(&ClusterNode::node)));
            } else {
                std::ranges::for_each(result, [&clusterResult](auto && clusterNode){
                    clusterResult.noise.push_back(std::move(clusterNode.node));
                });
            }
        }
        return clusterResult;
    }
};
}  //namespace fishnet