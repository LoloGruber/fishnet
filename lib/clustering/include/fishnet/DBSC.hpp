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
    struct ClusterNodeStats {
        double densityIndicator;
        double meanDistance;
        double nearestDistance;
    };

    double T1 = NAN; // attribute difference threshold for spatial reachability, will be computed from the data
    const double eps;
    const size_t beta;
    const size_t minPts;
    fishnet::util::BiFunction_t<T,T,double> distanceFunction;
    fishnet::util::UnaryFunction_t<T, double> attributeExtractor;
    std::unordered_map<size_t, bool> visitedNodes; // node id to visited flag
    std::unordered_map<size_t, ClusterNodeStats> clusterNodeStats; // node id to cluster node stats

    struct ClusterNode {
        T node;
        size_t id;
        double attributeValue;

        bool operator==(const ClusterNode & other) const {
            return this->id == other.id;
        }
    };

    struct ClusterNodeHash {
        size_t operator()(const ClusterNode & node) const {
            return node.id;
        }
    };

    auto ClusterNodeOrdering() const noexcept {
        return [this](const ClusterNode & lhs, const ClusterNode & rhs) {
            const auto &lhsStats = clusterNodeStats.at(lhs.id);
            const auto &rhsStats = clusterNodeStats.at(rhs.id);

            if (! fishnet::math::areEqual(lhsStats.densityIndicator, rhsStats.densityIndicator)) {
                return lhsStats.densityIndicator > rhsStats.densityIndicator;
            }
            return lhsStats.meanDistance < rhsStats.meanDistance;
        };
    }

    auto ClusterNodeOrderingReverse() const noexcept {
        return [ordering = ClusterNodeOrdering()](const ClusterNode & lhs, const ClusterNode & rhs) {
            return not ordering(rhs, lhs);
        };
    }

    double spatial_distance(const T & lhs, const T & rhs) const noexcept {
        return distanceFunction(lhs, rhs);
    }

    double inline spatial_distance(const ClusterNode & lhs, const ClusterNode & rhs) const noexcept {
        return distanceFunction(lhs.node, rhs.node);
    }

    bool inline spatially_directly_reachable(const ClusterNode & p, const ClusterNode & q)const noexcept {
        return spatial_distance(p, q) <= eps;
    }

    bool inline attribute_distance(const ClusterNode & p, const ClusterNode & q) const noexcept {
        return std::abs(p.attributeValue - q.attributeValue);
    }

    bool reachable(const ClusterNode & q, fishnet::util::forward_range_of<ClusterNode> auto && temporalCluster) {
        double avg_clu = fishnet::math::avg(temporalCluster, &ClusterNode::attributeValue).value_or_throw();
        return std::abs(q.attributeValue - avg_clu) <= T1;
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
        clusterGraph.addNodes(std::views::values(nodeMap));
        for(const auto & node: graph.getNodes()){
            for(const auto & nbr: graph.getNeighbours(node)){
                clusterGraph.addEdge(nodeMap.at(node), nodeMap.at(nbr));
            }
        }
        return clusterGraph;
    }

    double prepareClusteringData(fishnet::graph::Graph auto & graph) {
        /* Update cluster nodes in the trimmed graph with density indicators and mean attribute difference and nearest neighbour attribute difference */
        for(const ClusterNode & clusterNode : graph.getNodes()){
            visitedNodes[clusterNode.id] = false;
            auto neighbors = graph.getNeighbours(clusterNode);
            double N_size = static_cast<double>(fishnet::util::size(neighbors));
            if(N_size == 0) continue;
            double N_SDR = static_cast<double>(std::ranges::count_if(neighbors, [&](const auto & nbr) {
                return spatially_directly_reachable(clusterNode, nbr);
            }));
            const ClusterNode & nearestNeighbour = *std::ranges::min_element(neighbors, [&](const auto & lhs, const auto & rhs) {
                return spatial_distance(clusterNode, lhs) < spatial_distance(clusterNode, rhs);
            });
            double nearestDistance = attribute_distance(clusterNode, nearestNeighbour);
            double meanDistance = fishnet::math::mean(neighbors, [&](const auto & nbr) {
                return attribute_distance(clusterNode, nbr);
            }).value_or(0.0);
            double densityIndicator = N_SDR + N_SDR/N_size;
            clusterNodeStats[clusterNode.id] = ClusterNodeStats{
                .densityIndicator = densityIndicator,
                .meanDistance = meanDistance,
                .nearestDistance = nearestDistance
            };
        }
        double avgAttributeDiff = fishnet::math::mean(std::views::values(clusterNodeStats),&ClusterNodeStats::nearestDistance).value_or_throw("Illegal state, graph has at least one node, so attributeDiffs should not be empty");
        double stdAttributeDiff = fishnet::math::std(std::views::values(clusterNodeStats), &ClusterNodeStats::nearestDistance).value_or_throw("Illegal state, graph has at least one node, so attributeDiffs should not be empty"); 
        const double T1 = fishnet::math::mean(std::views::values(clusterNodeStats) | std::views::filter([avgAttributeDiff, stdAttributeDiff](const ClusterNodeStats & data) {
            return avgAttributeDiff - 3 * stdAttributeDiff <= data.nearestDistance && data.nearestDistance <= avgAttributeDiff + 3 * stdAttributeDiff;
        }), &ClusterNodeStats::nearestDistance).value_or_throw("Illegal state, there should be at least one non-outlier attribute difference, so the filtered range should not be empty");
        return T1;
    }

    bool is_expanding_core(const ClusterNode & node, fishnet::graph::Graph auto const & graph) const noexcept {
        bool visited = this->visited(node);
        size_t counter= std::ranges::count_if(graph.getNeighbours(node), [this,&node](const auto & nbr) {
            return spatially_directly_reachable(node, nbr);
        });
        return !visited && counter > 0;
    } 

    bool visited(const ClusterNode & node) const {
        return visitedNodes.at(node.id);
    }

    void markVisited(const ClusterNode & node) {
        visitedNodes[node.id] = true;
    }

    std::vector<ClusterNode> expand(const ClusterNode & core, fishnet::graph::Graph auto const& graph) {
        std::vector<ClusterNode> cluster;
        cluster.push_back(core);
        std::priority_queue<ClusterNode,std::vector<ClusterNode>,decltype(ClusterNodeOrderingReverse())> expandingCores(ClusterNodeOrderingReverse());
        expandingCores.push(core);
        while (!expandingCores.empty()) {
            const ClusterNode current = expandingCores.top();
            expandingCores.pop();
            for (const ClusterNode & nbr : graph.getNeighbours(current)) {
                if (!visited(nbr) && spatially_directly_reachable(current,nbr) && reachable(nbr, cluster)) {
                    if (is_expanding_core(nbr, graph)){
                        expandingCores.push(nbr);
                    }
                    markVisited(nbr);
                    cluster.push_back(nbr);
                }
            }
        }
        return cluster;
    }

public: 
    DBSC(double eps, size_t beta, size_t minPts, fishnet::util::BiFunction<T,T,double> auto && distanceFunction, fishnet::util::UnaryFunction<T, double> auto && attributeExtractor) 
        : eps(eps), beta(beta), minPts(minPts), distanceFunction(std::forward<fishnet::util::BiFunction_t<T,T,double>>(distanceFunction)), attributeExtractor(std::forward<fishnet::util::UnaryFunction_t<T, double>>(attributeExtractor)) {}

    DBSC(double eps, size_t beta, size_t minPts, fishnet::util::BiFunction<T,T,double> auto && distanceFunction) 
        :eps(eps), beta(beta), minPts(minPts), distanceFunction(std::forward<fishnet::util::BiFunction_t<T,T,double>>(distanceFunction)), attributeExtractor([](){return 0.0;}) {}

    void setCustomT1(double t1) {
        this->T1 = t1;
    }

    ClusterResult<T> operator()(fishnet::graph::Graph auto const & input_graph) {
        static_assert(std::derived_from<typename std::decay_t<decltype(input_graph)>::node_type, T>, "Graph node type must be compatible with DBSC point type");
        auto graph = getClusterGraph(input_graph);
        auto initalEdges = graph.getEdges();
        auto edgeDistanceMapper = [this](const auto & edge) {return spatial_distance(edge.getFrom(), edge.getTo());};
        const double global_mean = fishnet::math::mean(initalEdges, edgeDistanceMapper).value_or(0.0);
        const double global_variation = fishnet::math::std(initalEdges, edgeDistanceMapper, global_mean).value_or(0.0);
        auto local_mean = [&](const ClusterNode & node) -> double{
            return fishnet::math::mean(graph.getNeighbours(node), [this,&node](const auto & nbr) {
                return spatial_distance(node, nbr);
            }).value_or(0.0);
        };
        auto local_variation = [&](const ClusterNode & node) -> double{
            return fishnet::math::std(graph.getNeighbours(node), [this,&node](const auto & nbr) {
                return spatial_distance(node, nbr);
            }).value_or(0.0);
        };

        /* Remove global long edges */
        std::vector<std::pair<ClusterNode, ClusterNode>> edgesToRemove;
        for(const auto & node : graph.getNodes()){
            double global_distance_constraint = global_mean + (global_mean/local_mean(node)) * global_variation;
            for(const auto & nbr : graph.getNeighbours(node)){
                if(spatial_distance(node, nbr) > global_distance_constraint){
                    edgesToRemove.emplace_back(node, nbr);
                }
            }
        }
        std::ranges::for_each(edgesToRemove, [&graph](const auto & edge){
            graph.removeEdge(edge.first, edge.second);
        });

        edgesToRemove.clear();
        /* Remove local long edges*/
        for (const auto & node: graph.getNodes()){
            auto order2_graph = fishnet::graph::BFS::neighborhood(graph, node, 2);
            auto order2_mean = fishnet::math::mean(order2_graph.getEdges(), edgeDistanceMapper).value_or(0.0);
            auto mean_local_variation = fishnet::math::mean(order2_graph.getNodes(), local_variation).value_or(0.0);
            auto local_distance_constraint = order2_mean + static_cast<double>(beta) * mean_local_variation;
            for(const auto & edge: order2_graph.getEdges()){
                if(spatial_distance(edge.getFrom(), edge.getTo()) > local_distance_constraint){
                    edgesToRemove.emplace_back(edge.getFrom(), edge.getTo());
                }
            }
        }
        std::ranges::for_each(edgesToRemove, [&graph](const auto & edge){
            graph.removeEdge(edge.first, edge.second);
        });
        /* Prepare clustering data */
        double calculatedT1 =prepareClusteringData(graph);
        this->T1 = std::isnan(this->T1) ? calculatedT1 : this->T1;
        std::vector<ClusterNode> clusteringData(graph.getNodes().begin(), graph.getNodes().end());
        std::ranges::sort(clusteringData, ClusterNodeOrdering());
        assert(!std::isnan(this->T1) && "T1 should have been set either by user or calculated from the data, but it is still NaN, indicating an illegal state");
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

static_assert(ClusterAlgorithm<DBSC<int>, fishnet::graph::UndirectedGraph<int>>);

template<typename T>
struct DBSCBuilder {
    double eps;
    size_t beta;
    size_t minPts = 2;
    fishnet::util::BiFunction_t<T,T,double> distanceFunction;
    fishnet::util::UnaryFunction_t<T, double> attributeExtractor = [](const auto & node){return 0.0;};
    double t1 = NAN;

    auto build() {
        DBSC<T> dbsc(eps, beta, minPts, distanceFunction, attributeExtractor);
        if(!std::isnan(t1)){
            dbsc.setCustomT1(t1);
        }
        return dbsc;
    }

    DBSCBuilder<T> & setEps(double eps) {
        this->eps = eps;
        return *this;
    }

    DBSCBuilder<T> & setBeta(size_t beta) {
        this->beta = beta;
        return *this;
    }

    DBSCBuilder<T> & setMinPts(size_t minPts) {
        this->minPts = minPts;
        return *this;
    }

    DBSCBuilder<T> & setDistanceFunction(fishnet::util::BiFunction<T,T,double> auto && distanceFunction) {
        this->distanceFunction = distanceFunction;
        return *this;
    }

    DBSCBuilder<T> & setAttributeExtractor(fishnet::util::UnaryFunction<T, double> auto && attributeExtractor) {
        this->attributeExtractor = attributeExtractor;
        return *this;
    }

    DBSCBuilder<T> & setT1(double t1) {
        this->t1 = t1;
        return *this;
    }

};

}  //namespace fishnet