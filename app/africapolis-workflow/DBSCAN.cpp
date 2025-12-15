#include <fishnet/Fishnet.hpp>
#include <fishnet/TaskConfig.hpp>
#include <CLI/CLI.hpp>
#include <fishnet/SettlementPolygon.hpp>

class DBSCAN {
private:
    double eps;
    int minPts;


    template<fishnet::geometry::GeometryObject G> 
    void expandCluster(fishnet::graph::Graph<G> auto const & graph, 
                    const G& point,
                    fishnet::util::range_of<G> auto neighbors, int clusterID,
                    std::unordered_map<G, int>& labels) {
        std::queue<G> toProcess;
        labels[point] = clusterID;
        for (const auto& neighbor : neighbors) {
            toProcess.push(neighbor);
        }
        while (!toProcess.empty()) {
            G current = toProcess.front();
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

    template<fishnet::geometry::GeometryObject G>
    auto regionQuery(fishnet::graph::Graph<G> auto const & graph, const G & point) {
        return graph.getNeighbours(point) | std::views::filter([&](const G& neighbor) {
            return point.distance(neighbor) <= eps;
        });
    }

public:
    DBSCAN(double eps, int minPts) : eps(eps), minPts(minPts) {}


    template<fishnet::geometry::GeometryObject G>
    std::vector<std::vector<G>> run(fishnet::graph::Graph<G> auto const & graph) {
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
        int outputSize = clusterID +1;
        std::vector<std::vector<G>> result;
        result.resize(outputSize);
        for (auto && pair : labels) {
            result[pair.second].push_back(std::move(pair.first));
        }
        return result;
    }
};


int main(int argc, char *argv[]){
    CLI::App app{"Fishnet DBSCAN Clustering Algorithm"};
    std::string inputfile;
    std::string configfile;
    double eps;
    int minPts;
    app.add_option("-i,--inputs", inputfile, "Input file path")->required();
    app.add_option("-c,--config", configfile, "Workflow configuration file path")->required();
    app.add_option("-e,--eps", eps, "Epsilon distance")->required();
    app.add_option("-m,--minPts", minPts, "Minimum points to form a cluster")->required();
    CLI11_PARSE(app, argc, argv);   
    MemgraphTaskConfig config(nlohmann::json::parse(std::ifstream(configfile)));


    return 0;
}
