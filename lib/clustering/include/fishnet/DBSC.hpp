#include <fishnet/Graph.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/Statistics.hpp>
#include <fishnet/Constants.hpp>
#include <ranges>
#include <unordered_map>
#include <cmath>
#include "ClusterAlgorithm.hpp"


namespace fishnet {

template<typename T>
class DBSC {
private:
    const double eps;
    const int beta;
    fishnet::util::BiFunction_t<T,T,double> distanceFunction;
    fishnet::util::UnaryFunction_t<T, double> attributeExtractor;

    struct ClusterNode {
        T node;
        size_t id;
        double attributeValue;
        mutable double densityIndicator = 0.0;
        mutable double meanAttributeDiff = 0.0; // mean attribute difference to neighbours
        mutable double nearestAttributeDiff = 0.0; // attribute difference to nearest neighbour
        mutable bool visited = false;

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
        std::unordered_map<const T*, ClusterNode> nodeMap;
        for(const auto & node : graph.getNodes()){
            nodeMap.try_emplace(&node, ClusterNode{
                .node = node,
                .id = nodeIndex++,

                .attributeValue = attributeExtractor(node)
            });
        }
        for(const auto & node: graph.getNodes()){
            for(const auto & nbr: graph.getNeighbours(node)){
                clusterGraph.addEdge(nodeMap[&node], nodeMap[&nbr]);
            }
        }
        return clusterGraph;
    }

    double prepareClusteringData(fishnet::graph::Graph auto & graph) {
        /* Update cluster nodes in the trimmed graph with density indicators and mean attribute difference and nearest neighbour attribute difference */
        auto nodes = graph.getNodes();
        for(const ClusterNode & clusterNode : nodes){
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
        const double T1 = prepareClusteringData(graph);
        std::vector<ClusterNode> clusteringData(graph.getNodes().begin(), graph.getNodes().end());
        std::ranges::sort(clusteringData, ClusterNodeOrdering{});
        auto count_spatially_directly_reachable = [this,&graph](const ClusterNode & node) {
            return std::ranges::count_if(graph.getNeighbours(node), [this,&node](const auto & nbr) {
                return spatially_directly_reachable(node, nbr);
            });
        };
        auto reachable = [this,T1](const ClusterNode & core, const ClusterNode & q, fishnet::util::forward_range_of<ClusterNode> auto && temporalCluster) {
            double avg_clu = core.attributeValue;
            for(const auto & node : temporalCluster){
                avg_clu += node.attributeValue;
            }
            avg_clu = avg_clu/(1 + static_cast<double>(fishnet::util::size(temporalCluster)));
            return std::abs(q.attributeValue - avg_clu) <= T1;
        };
        /* Clustering */
        ClusterResult<T> clusterResult;
        for(const ClusterNode & core : clusteringData){
            if(core.visited) 
                continue;
            core.visited = true;
            std::vector<T> cluster;
            cluster.push_back(core.node);
            /* Collect expanding cores (i.e. neighbours that directly spatially reach neighbour neighbours) */
            std::vector<ClusterNode> expandingCores;
            for (const ClusterNode & nbr : graph.getNeighbours(core)) {
                if(nbr.visited || count_spatially_directly_reachable(nbr) == 0)
                    continue;
                expandingCores.push_back(nbr); 
            }
            std::ranges::sort(expandingCores, ClusterNodeOrdering{});
            
  
            // (ii) add the expanding clusters if they are spatially reachable and spatially directly reachable
            for (const auto &expandingCore : expandingCores) {
                if (spatially_directly_reachable(core,expandingCore) && reachable(core, expandingCore, expandingCores)) {
                    expandingCore.visited = true;
                    cluster.push_back(expandingCore.node);
                }
            }
            
            // (iii) expand using k-order neighbors (k = beta)
            auto betaOrderGraph = fishnet::graph::BFS::neighborhood(graph,core,beta);
            // find first starting expanding core (highest density)
            //std::vector<PointType> k_order_graph_nodes(k_order_neighbors.getNodes().begin(), k_order_neighbors.getNodes().end());
            // std::list<PointType> ordered_candidates = descending_density(graph, k_order_neighbors.nodes, attributeExtractor, eps);
            
            // for (auto &starting_core : ordered_candidates) {
            //     if (visited[starting_core]) continue;
            //     if (number_of_SDR(graph, starting_core, attributeExtractor, eps) == 0) continue;
            //     // treat starting_core as new "seed"
            //     std::vector<PointType> local_expanding;
            //     local_expanding.push_back(starting_core);
            //     visited[starting_core] = true;
            //     cluster.push_back(starting_core);
            //     bool expanded = true;
            //     while (expanded) {
            //         expanded = false;
            //         // gather expanding cores around current seed
            //         std::vector<PointType> new_candidates;
            //         for (auto nbr : graph.getNeighbours(starting_core)) {
            //             double number = number_of_SDR(graph, nbr, attributeExtractor, eps);
            //             if (!visited[nbr] && number > 0)
            //             {
            //                 new_candidates.push_back(nbr);
            //             }
            //         }
            //         // rank by density
            //         std::list<PointType> ordered_new =
            //             descending_density(graph, new_candidates, attributeExtractor, eps);
            //         for (auto &cand : ordered_new) {
            //             if (!visited[cand] &&
            //                 spatially_directly_reachable(graph, core, cand, attributeExtractor, eps) &&
            //                 spatially_reachable(cluster, graph, attributeExtractor, core, cand, eps))
            //             {
            //                 cluster.push_back(cand);
            //                 visited[cand] = true;
            //                 expanded = true;
            //             }
            //         }
            //     }
            // }
            
            // // if only the core is in the cluster, then it should be marked as noise
            // if (cluster.size() > 1) {
            //     clusters[cluster_id++] = cluster;
            // }
            // else {
            //     visited[core] = false;

            // }

        }







        return ClusterResult<T>();
    }
    


    // std::unordered_map<int, std::vector<PointType>> get_clusters(GraphT&graph,
    //     fishnet::util::UnaryFunction<PointType, double> auto & attributeExtractor, double eps, int beta)
    // {
    //     std::unordered_map<PointType, bool> visited;     // track classified/unclassified
    //     for (auto &node : graph.getNodes()) {
    //         visited[node] = false;
    //     }
    //     std::unordered_map<int, std::vector<PointType>> clusters;
    //     int cluster_id = 0;
    //     auto nodesView = graph.getNodes();
    //     std::vector<PointType> graph_nodes(nodesView.begin(), nodesView.end());
    //     std::list<PointType> density_order = descending_density(graph, graph_nodes, attributeExtractor, eps);
    //     for (auto &core : density_order) {
    //         if (visited[core]) continue; 
    //         std::vector<PointType> cluster;
    //         cluster.push_back(core);
    //         visited[core] = true;
    //         // (i)
    //         // collect expanding cores of this core
    //         std::vector<PointType> expanding_cores;
    //         for (auto nbr : graph.getNeighbours(core)) {
    //             double nr_sdr = number_of_SDR(graph, nbr, attributeExtractor, eps);
    //             if (!visited[nbr] && nr_sdr > 0)

    //             {
    //                 expanding_cores.push_back(nbr);
    //             }
    //         }
    //         // (i) rank the expanding cores according to their density indicator
    //         std::list<PointType> ordered_expanding = descending_density(graph, expanding_cores, attributeExtractor, eps);
           
    //         // (ii) add the expanding clusters if they are spatially reachable and spatially directly reachable
    //         for (auto &ecore : ordered_expanding) {
    //             if (spatially_directly_reachable(graph, core, ecore, attributeExtractor, eps) && spatially_reachable(cluster, graph, attributeExtractor, core, ecore, eps)) {
    //                 cluster.push_back(ecore);
    //                 visited[ecore] = true;
    //             }
    //         }
            
    //         // (iii) expand using k-order neighbors (k = beta)
    //         using EdgeType = fishnet::graph::__impl::BaseEdge<PointType, false>;
    //         auto k_order_neighbors = beta_order_subgraph<PointType, GraphT, EdgeType>(graph, core, beta);
    //         // find first starting expanding core (highest density)
    //         //std::vector<PointType> k_order_graph_nodes(k_order_neighbors.getNodes().begin(), k_order_neighbors.getNodes().end());
    //         std::list<PointType> ordered_candidates = descending_density(graph, k_order_neighbors.nodes, attributeExtractor, eps);
            
    //         for (auto &starting_core : ordered_candidates) {
    //             if (visited[starting_core]) continue;
    //             if (number_of_SDR(graph, starting_core, attributeExtractor, eps) == 0) continue;
    //             // treat starting_core as new "seed"
    //             std::vector<PointType> local_expanding;
    //             local_expanding.push_back(starting_core);
    //             visited[starting_core] = true;
    //             cluster.push_back(starting_core);
    //             bool expanded = true;
    //             while (expanded) {
    //                 expanded = false;
    //                 // gather expanding cores around current seed
    //                 std::vector<PointType> new_candidates;
    //                 for (auto nbr : graph.getNeighbours(starting_core)) {
    //                     double number = number_of_SDR(graph, nbr, attributeExtractor, eps);
    //                     if (!visited[nbr] && number > 0)
    //                     {
    //                         new_candidates.push_back(nbr);
    //                     }
    //                 }
    //                 // rank by density
    //                 std::list<PointType> ordered_new =
    //                     descending_density(graph, new_candidates, attributeExtractor, eps);
    //                 for (auto &cand : ordered_new) {
    //                     if (!visited[cand] &&
    //                         spatially_directly_reachable(graph, core, cand, attributeExtractor, eps) &&
    //                         spatially_reachable(cluster, graph, attributeExtractor, core, cand, eps))
    //                     {
    //                         cluster.push_back(cand);
    //                         visited[cand] = true;
    //                         expanded = true;
    //                     }
    //                 }
    //             }
    //         }
            
    //         // if only the core is in the cluster, then it should be marked as noise
    //         if (cluster.size() > 1) {
    //             clusters[cluster_id++] = cluster;
    //         }
    //         else {
    //             visited[core] = false;

    //         }
    //     }
    //     // step (v): assign noise 
    //     std::vector<PointType> noise;
    //     for (auto &[node, was_visited] : visited) {
    //         if (!was_visited) {
    //             noise.push_back(node);
    //         }
    //     }
    //     if (!noise.empty()) {
    //         clusters[-1] = noise;
    //     }
    //     return clusters;
    // }
    // // if Q is spatially directly reachable from Pi 
    // bool spatially_directly_reachable(
    //     GraphT&graph, 
    //     const PointType &point, 
    //     PointType &Q, 
    //     fishnet::util::UnaryFunction<PointType, double> auto & attributeExtractor,
    //     double eps) {
    //     // (i) Q in Neighbors(point) and  
    //     // (ii) attr_diff(P, Q) <= T_1, treshold = eps
    //     const auto neighbours = graph.getNeighbours(point);
    //     bool inNeighbours =  (std::find(neighbours.begin(), neighbours.end(), Q) != neighbours.end());
    //     double attrDiff = std::abs(attributeExtractor(point) - attributeExtractor(Q));
    //     bool epsThreshold = attrDiff <= eps;
       
    //     return inNeighbours && epsThreshold;
    // }
    // bool spatially_reachable(std::vector<PointType>& cluster, const GraphT & graph, 
    //     fishnet::util::UnaryFunction<PointType, double> auto & attributeExtractor, PointType& point, PointType& Q, double eps){
    //     // if Q is spatially reachable from Pi
    //     // given set of spatial objects CLU
    //     // (i) attr_diff(Qi, Avg(CLU)) <= T1 and
    //     // (ii) Qi in Neighbors(Pi) and Pi in CLU
    //     double avg_clusters = 0.0;
    //     for(auto node : cluster){
    //         avg_clusters += attributeExtractor(node);
    //     }
    //     avg_clusters = avg_clusters/cluster.size();
    //     double attr_diff =  std::abs(attributeExtractor(Q) - avg_clusters);
    //     const auto neighbours = graph.getNeighbours(point);
    //     bool inNeighbours =  (std::find(neighbours.begin(), neighbours.end(), Q) != neighbours.end());
    //     bool inCLU =  (std::find(cluster.begin(), cluster.end(), point) != cluster.end());
    //     bool result = inNeighbours && inCLU && (attr_diff <= eps);
    //     return result;
    // }
    // //  sort the objects in order of descending density indicator
    // std::list<PointType> descending_density(GraphT& graph, std::vector<PointType> &nodes, fishnet::util::UnaryFunction<PointType, double> auto & attributeExtractor, double eps){
        
    //     std::vector<PointType> sorted_nodes = nodes; 
    //     // suppose time complexity of sort() is O(N log(N))
    //     std::sort(sorted_nodes.begin(), sorted_nodes.end(),[&](const PointType &a, const PointType &b) {
    //         double dens_a = density_indicator(graph, a, attributeExtractor, eps);
    //         double dens_b = density_indicator(graph, b, attributeExtractor, eps);
    //         return dens_a > dens_b;
    //     });
    //     std::list<PointType> result(sorted_nodes.begin(), sorted_nodes.end());
    //     return result;
    // }
};
} // namespace fishnet