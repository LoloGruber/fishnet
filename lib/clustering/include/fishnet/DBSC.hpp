#include <fishnet/Graph.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/Statistics.hpp>
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

    double distance(const T & lhs, const T & rhs) {
        return distanceFunction(lhs, rhs);
    }

public: 
    DBSC(double eps, int beta, fishnet::util::BiFunction<T,T,double> auto && distanceFunction, fishnet::util::UnaryFunction<T, double> auto && attributeExtractor) 
        : eps(eps), beta(beta), distanceFunction(std::forward<fishnet::util::BiFunction_t<T,T,double>>(distanceFunction)), attributeExtractor(std::forward<fishnet::util::UnaryFunction_t<T, double>>(attributeExtractor)) {}

    auto cache_distances(fishnet::graph::Graph auto const & graph){
        std::unordered_map<T, std::unordered_map<T, double>> distanceMap;
        for (const auto & node : graph.getNodes()) {
            std::unordered_map<T, double> nbrs_distances;
            for (const auto & nbr : graph.getNeighbours(node)) {
                nbrs_distances[nbr] = distanceFunction(node, nbr);
            }
            distanceMap[node] = nbrs_distances;
        }
        return distanceMap;
    }

    auto meanEdgeDistance(std::ranges::forward_range auto const & edges){
        static_assert(fishnet::graph::Edge<std::ranges::range_value_t<decltype(edges)>,T>, "Input range must be a range of graph edges");
        return fishnet::math::mean(edges, [&](const auto & edge) {
            return distance(edge.getFrom(), edge.getTo());
        }).value_or(0.0);
    }

    auto stdEdgeDistance(std::ranges::forward_range auto const & edges, double mean){
        static_assert(fishnet::graph::Edge<std::ranges::range_value_t<decltype(edges)>,T>, "Input range must be a range of graph edges");
        return fishnet::math::std(edges, [&](const auto & edge) {
            return distance(edge.getFrom(), edge.getTo());
        }, mean).value_or(0.0);
    }

    auto cluster(fishnet::graph::Graph auto & graph) {
        static_assert(std::derived_from<typename std::decay_t<decltype(graph)>::node_type, T>, "Graph node type must be compatible with DBSC point type");
        const double global_mean = meanEdgeDistance(graph.getEdges());
        const double global_variation = stdEdgeDistance(graph.getEdges(), global_mean);
        auto local_mean = [&](const T & node) -> double{
            return fishnet::math::mean(graph.getNeighbours(node), [this,&node](const auto & nbr) {
                return distance(node, nbr);
            }).value_or(0.0);
        };
        auto local_variation = [&](const T & node) -> double{
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
            auto order2_mean = meanEdgeDistance(order2_graph.getEdges());
            auto mean_local_variation = fishnet::math::mean(order2_graph.getNodes(), local_variation).value_or(0.0);
            auto local_distance_constraint = order2_mean + beta * mean_local_variation;
            for(const auto & edge: order2_graph.getEdges()){
                if(distance(edge.getFrom(), edge.getTo()) > local_distance_constraint){
                    graph.removeEdge(edge);
                }
            }
        }

        auto density_indicator_attribute_diff = [&graph, this](const T & node) -> std::pair<double, double>{
            auto neighbors = graph.getNeighbours(node);
            double N_size = static_cast<double>(fishnet::util::size(neighbors));
            if(N_size == 0) {
                return {0, 0};
            }
            double N_SDR = std::ranges::count_if(neighbors, [&](const auto & nbr) {
                return distance(node, nbr) <= eps;
            });
            const T & nearestNeighbour = *std::ranges::min_element(neighbors, [&](const auto & lhs, const auto & rhs) {
                return distance(node, lhs) < distance(node, rhs);
            });
            double attribute_diff = std::abs(attributeExtractor(node) - attributeExtractor(nearestNeighbour));
            return {N_SDR + N_SDR/N_size, attribute_diff};
        };

        std::unordered_map<T, double> densityMap;
        std::vector<double> attributeDiffs;
        attributeDiffs.reserve(fishnet::util::size(graph.getNodes()));
        for(const auto & node : graph.getNodes()){
            auto [density, nearest_attribute_diff] = density_indicator_attribute_diff(node);
            densityMap[node] = density;
            attributeDiffs.push_back(nearest_attribute_diff);
        }
        double meanAttributeDiff = fishnet::math::mean(attributeDiffs).value_or_throw("Illegal state, graph has at least one node, so attributeDiffs should not be empty");
        double stdAttributeDiff = fishnet::math::std(attributeDiffs, meanAttributeDiff).value_or_throw("Illegal state, graph has at least one node, so attributeDiffs should not be empty"); 
        const double T1 = fishnet::math::mean(attributeDiffs | std::views::filter([meanAttributeDiff, stdAttributeDiff](double diff){
            return meanAttributeDiff - 3 * stdAttributeDiff <= diff && diff <= meanAttributeDiff + 3 * stdAttributeDiff;
        })).value_or_throw("Illegal state, there should be at least one non-outlier attribute difference, so the filtered range should not be empty");







        return ClusterResult<T>();


/*         // step 1
        auto removed_global_edges =  remove_global_long_edges(graph, distanceMap); 
        auto final_graph =  remove_local_edges(removed_global_edges, beta, distanceMap);
        // step 2
        auto nodesView_final = final_graph.getNodes();
        std::vector<PointType> final_graph_nodes(nodesView_final.begin(), nodesView_final.end());
        auto removed_outliers_graph = remove_outliers_three_sigma(final_graph, attributeExtractor, distanceMap);
        // time complexity of determination of T1 is O(N)
        // after removing the outliers, average attribute difference should be computed and assigned to eps
        double new_treshold =  mean_attribute_difference(removed_outliers_graph,attributeExtractor, distanceMap);
        const double eps = new_treshold;
        std::cout << "eps value: " <<  eps << std::endl;
        // step 3 
        auto clusters = get_clusters(graph, attributeExtractor, eps, beta);
        
        return clusters; */
    }
    
    
    // double mean_attribute_difference(GraphT&graph, 
    //     fishnet::util::UnaryFunction<PointType, double> auto & attributeExtractor, std::unordered_map<PointType, std::unordered_map<PointType, double>> distanceMap){
    //     double sum = 0.0;
    //     for(auto node : graph.getNodes()){
    //         double diff = attribute_difference(graph, attributeExtractor, node, distanceMap);
    //         sum+= diff;
    //     }
    //     if(fishnet::util::size(graph.getNodes()) == 0 || sum == 0) {return 0;}
    //     return sum/fishnet::util::size(graph.getNodes());
    // }
    // double number_of_SDR(GraphT&graph, const PointType point, fishnet::util::UnaryFunction<PointType, double> auto & attributeExtractor, double eps){
    //     double N_spr = 0;                              // number of spatially directly reachable objects from the point
    //     for (auto node : graph.getNeighbours(point)){
    //         if(spatially_directly_reachable(graph, point, node, attributeExtractor, eps)){N_spr +=1;}
    //     }
    //     return N_spr;
    // }

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
    // double attribute_standard_variation(GraphT&graph, 

    //     fishnet::util::UnaryFunction<PointType, double> auto & attributeExtractor, std::unordered_map<PointType, std::unordered_map<PointType, double>> distanceMap){
    //     double att_mean = mean_attribute_difference(graph, attributeExtractor, distanceMap);
    //     double variance = 0.0;
    //     int size = fishnet::util::size(graph.getNodes());
    //     for (auto node : graph.getNodes()){
    //         double attr_diff_value = attribute_difference(graph, attributeExtractor, node, distanceMap);
    //         variance+= (attr_diff_value - att_mean)*(attr_diff_value - att_mean);
    //     }
    //     variance /= size;
    //     return sqrt(variance);
    // }
    // double attribute_difference(
    //     GraphT&graph, 
    //     fishnet::util::UnaryFunction<PointType, double> auto & attributeExtractor, 
    //     PointType node, 
    //     std::unordered_map<PointType, std::unordered_map<PointType, double>> distanceMap){
    //     // find nearest neighbor of node
    //     std::optional<PointType> nearest;
    //     double minDist = std::numeric_limits<double>::max();
    //     for (const auto &nbr : graph.getNeighbours(node)) {
    //         double dist = distanceMap[node][nbr];
    //         if (dist < minDist) {
    //             minDist = dist;
    //             nearest = nbr;
    //         }
    //     }
    //     if (nearest.has_value()) {
            
    //         double diff = std::abs(attributeExtractor(node) - attributeExtractor(nearest.value()));
    //         return diff;
    //     }
        
    //     return 0.0;     //// if no neighbor, define difference as 0
    // }
    // GraphT remove_outliers_three_sigma(GraphT&graph, fishnet::util::UnaryFunction<PointType, double> auto & attributeExtractor,  
    //     std::unordered_map<PointType, std::unordered_map<PointType, double>> distanceMap){
    //         auto filtered_graph = graph;
    //         double mean = mean_attribute_difference(graph, attributeExtractor, distanceMap);
    //         double sigma = attribute_standard_variation(graph, attributeExtractor, distanceMap);
    //         // valid interval: mean - 3*sigma <= diff <= mean + 3*sigma
    //         for(auto node: graph.getNodes()){
    //             // compute on the unchanged graph 
    //             auto attr_diff = attribute_difference(graph, attributeExtractor, node, distanceMap);
    //             if( !(mean - 3* sigma <= attr_diff && attr_diff <= mean + 3 * sigma) ){
    //                 filtered_graph.removeNode(node);
    //             }
    //         }
    //     return filtered_graph;
    // }

    // double global_distance_constraint(GraphT&graph, PointType &point, double global_mean,double global_variation,  std::unordered_map<PointType, std::unordered_map<PointType, double>> distanceMap) { 
    //     double alpha = global_mean/local_Mean(graph, point, distanceMap);
    //     double global_dinst_constr = global_mean + alpha * global_variation;
    //     return global_dinst_constr;
    // }
    // GraphT remove_global_long_edges(GraphT& graph,  std::unordered_map<PointType, std::unordered_map<PointType, double>> distanceMap){
    //     GraphT graph_modified = graph;
    //     double global_mean = global_Mean(graph, distanceMap);
    //     double global_variation = global_Variation(graph, global_mean, distanceMap);
    //     std::unordered_map<PointType, double> gdc;
    //     for (auto node : graph.getNodes()){
    //         gdc[node] = global_distance_constraint(graph, node, global_mean,global_variation, distanceMap);
    //     }
    //     //O(n * (number of neighbours)) time complexity
    //     for (const auto &node : graph.getNodes()) {
    //         // Remove edges violating constraint
    //         auto node_distances = distanceMap[node];
    //         for ( auto neighbor : graph.getNeighbours(node)){
    //             if(node_distances[neighbor]> gdc[node]){
    //                 graph_modified.removeEdge(node, neighbor);
    //             }
    //         }
            
    //     }
    //     return graph_modified;
    // }

    // double local_distance_constraint(GraphT& graph, PointType &point, int beta, std::unordered_map<PointType, std::unordered_map<PointType, double>> distanceMap){
    //     double sum_edges = 0.0;
    //     using EdgeType = fishnet::graph::__impl::BaseEdge<PointType, false>;
    //     auto subgraph = beta_order_subgraph<PointType, GraphT, EdgeType>(graph, point, beta);
    //     for (auto edge : subgraph.edges){
    //         double dist = distanceMap[edge.getFrom()][edge.getTo()];
    //         sum_edges += dist;
    //     }
    //     // mean length of all edges that belong to a path of ≤ beta edges starting from Pi
    //     double beta_order_mean = sum_edges/fishnet::util::size(subgraph.edges);
    //     if(fishnet::util::size(subgraph.edges) == 0){beta_order_mean = 0;}
    //     double varaince_sum = 0.0;
    //     for(auto node : subgraph.nodes){
    //         varaince_sum += local_Variation(graph, node, distanceMap);
    //     }
    //     double mean_variation = varaince_sum/fishnet::util::size(subgraph.nodes);
    //     double local_distance_constraint = beta_order_mean + beta * mean_variation;
    //     return local_distance_constraint;
    // }
    // GraphT remove_local_edges(GraphT& graph, int beta, std::unordered_map<PointType, std::unordered_map<PointType, double>> distanceMap ){
    //     GraphT graph_modified = graph;
    //     // subgraph and LDC should be computed with the unchanged graph
    //     int countsubgraphs = 0;
    //     for (auto node : graph.getNodes()){
    //         std::cout << "countsubgraphs: " << countsubgraphs << std::endl;
    //         double LDC = local_distance_constraint(graph, node, beta, distanceMap);
    //         using EdgeType = fishnet::graph::__impl::BaseEdge<PointType, false>;
    //         auto subgraph = beta_order_subgraph<PointType, GraphT, EdgeType>(graph, node, beta);
    //         for (auto edge : subgraph.edges){
    //             double dist_neighbour = distanceMap[edge.getFrom()][edge.getTo()];
    //             if(dist_neighbour > LDC){
    //                 graph_modified.removeEdge(edge.getFrom(), edge.getTo());
    //             }
    //         }
    //         countsubgraphs ++;
    //     }
    //     return graph_modified;
    // } */
};
} // namespace fishnet