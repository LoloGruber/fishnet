#pragma once
#include <assert>
#include <fishnet/Graph.hpp>

class DBSCAN {
private:
    double eps;
    size_t minPts;

    template<fishnet::graph::Graph GraphType>
    void expandCluster(const GraphType & graph, 
                    const typename GraphType::node_type & point,
                    fishnet::util::range_of<typename GraphType::node_type> auto neighbors, int clusterID,
                    std::unordered_map<typename GraphType::node_type, int>& labels) {
        std::queue<typename GraphType::node_type> toProcess;
        labels[point] = clusterID;
        for (const auto& neighbor : neighbors) {
            toProcess.push(neighbor);
        }
        while (!toProcess.empty()) {
            auto current = toProcess.front();
            toProcess.pop();
            if(labels.contains(current) && labels[current] != -1) {
                continue;
            }
            labels[current] = clusterID;
            auto neighborNeighbors = regionQuery(graph, current);
            if (fishnet::util::size(neighborNeighbors) >= minPts) {
                for (const auto& newNeighbor : neighborNeighbors) {
                    toProcess.push(newNeighbor);     
                }
            }
            
        }
    }

    template<fishnet::graph::Graph GraphType>
    auto regionQuery(const GraphType & graph, const typename GraphType::node_type & point) {
        return graph.getNeighbours(point) | std::views::filter([&](const auto & neighbor) {
            return point.distance(neighbor) <= eps;
        });
    }

public:
    DBSCAN(double eps, size_t minPts) : eps(eps), minPts(minPts) {
        assert(eps > 0,"Epsilon must be positive");
        assert(minPts > 0,"Minimum Points per Cluster must be positive");
    }

    template<fishnet::geometry::GeometryObject G>
    struct ClusterResult {
        std::vector<std::vector<G>> clusters;
        std::vector<G> noise;
    };


    auto cluster(fishnet::graph::Graph auto const & graph) {
        using G = typename std::decay_t<decltype(graph)>::node_type;
        int clusterID = 0;
        std::unordered_map<G, int> labels;
        for (const auto& node : graph.getNodes()) {
            if(labels.contains(node)) {
                continue;
            }
            auto neighbors = regionQuery(graph, node);
            if (fishnet::util::size(neighbors) < minPts) {
                labels[node] = -1;
                continue;
            } 
            expandCluster(graph, node, neighbors, clusterID, labels);
            clusterID++;
        }
        auto vals = labels | std::views::values;
        auto maxLabel = std::ranges::max_element(vals);
        if(maxLabel == vals.end()){
            return ClusterResult<G>{};
        }
        if(*maxLabel == -1){
            ClusterResult<G> result;
            for(auto && pair : labels) {
                result.noise.push_back(std::move(pair.first));
            }
            return result;
        }
        ClusterResult<G> result;
        result.clusters.resize(*maxLabel + 1);
        for (auto && pair : labels) {
            if(pair.second == -1){
                result.noise.push_back(std::move(pair.first));
                continue;
            }
            result.clusters[pair.second].push_back(std::move(pair.first));
        }
        return result;
    }
};