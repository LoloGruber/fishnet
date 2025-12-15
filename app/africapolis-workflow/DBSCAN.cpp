#include <fishnet/Fishnet.hpp>
#include <fishnet/TaskConfig.hpp>
#include <CLI/CLI.hpp>
#include <fishnet/SettlementShape.hpp>
#include <fishnet/IDReduceFunction.hpp>
#include <fishnet/CachingMemgraphAdjacency.hpp>
#include <fishnet/Task.hpp>

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
    DBSCAN(double eps, size_t minPts) : eps(eps), minPts(minPts) {}

    template<fishnet::geometry::GeometryObject G>
    struct ClusterResult {
        std::vector<std::vector<G>> clusters;
        std::vector<G> noise;
    };


    auto run(fishnet::graph::Graph auto const & graph) {
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
        int outputSize = clusterID +1;
        ClusterResult<G> result;
        result.clusters.resize(outputSize);
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
template<fishnet::geometry::GeometryObject G>
class ObservableShapefileReader {
public:
    using geometry_type = G;
    using file_type = fishnet::Shapefile;

private:
    OGRSpatialReference spatialRef;
    fishnet::util::Consumer_t<fishnet::VectorLayer<G>> onSuccess;
public:
    ObservableShapefileReader(fishnet::util::Consumer<fishnet::VectorLayer<G>> auto && onSuccess)
    : onSuccess(std::move(onSuccess)) {}

    fishnet::util::Either<fishnet::VectorLayer<G>,std::string> operator()(const fishnet::Shapefile & shapefile) const {
        auto layer = fishnet::VectorIO::tryRead(fishnet::ShapefileReader<G>{},shapefile);
        if(layer)
            onSuccess(layer.value());
        return layer;
    }

    OGRSpatialReference getSpatialReference() const noexcept {
        return spatialRef;
    }
};


int main(int argc, char *argv[]){
    CLI::App app{"Fishnet DBSCAN Clustering Algorithm"};
    std::vector<std::string> inputfiles;
    std::vector<ComponentReference> components;
    std::string configfile;
    double eps;
    int minPts;
    app.add_option("-i,--inputs",inputfiles,"Input Shapefiles storing the polygons with id for clustering")->required()->each([](const std::string & str){
        try{
            auto file = fishnet::Shapefile(str);
            if(not file.exists())
                throw std::invalid_argument("File "+ file.getPath().string() + " does not exist");
        }catch(std::invalid_argument & error){
            throw CLI::ValidationError(error.what());
        }
    });
    app.add_option("-c,--config", configfile, "Workflow configuration file path")->required();
    app.add_option("--components",components,"Component ids of connected components to contract")->each([](const std::string & str){
        std::stringstream stringStream {str};
        ComponentReference compRef;
        decltype(compRef.componentId) id = 0;
        if(not stringStream >> id){
            throw CLI::ValidationError("Could not parse \""+str+"\" to a component id");
        }
    });
    app.add_option("-e,--eps", eps, "Epsilon distance")->required();
    app.add_option("-m,--minPts", minPts, "Minimum points to form a cluster")->required();
    CLI11_PARSE(app, argc, argv);   
    using ShapeType = fishnet::geometry::Polygon<double>;
    using SettlementType = SettlementShape<ShapeType>;
    MemgraphTaskConfig config(nlohmann::json::parse(std::ifstream(configfile)));
    MemgraphConnection dbConnection = MemgraphConnection::create(config.params).value_or_throw();
    auto memgraphAdj = CachingMemgraphAdjacency<SettlementType>(std::move(dbConnection));
    auto shapeFiles = inputfiles | std::views::transform([](const std::string & str){ return fishnet::Shapefile(str); });
    OGRSpatialReference spatialRef;
    auto onReadStoreSpatialRef = [&spatialRef](const fishnet::VectorLayer<ShapeType> & layer){
        if(spatialRef.IsEmpty()){
            spatialRef = layer.getSpatialReference();
        }
    };
    auto fileRefMapper = [&memgraphAdj](const fishnet::Shapefile & shp){
        auto fileRef = memgraphAdj.getDatabaseConnection().addFileReference(shp.getPath());
        if(not fileRef){
            throw std::runtime_error("Could not read file reference for shp file:\n"+shp.getPath().string());
        }
        return fileRef.value();
    };
    ObservableShapefileReader<ShapeType> reader(onReadStoreSpatialRef);
    auto settlements = SettlementType::read<fishnet::Shapefile>(shapeFiles, reader,fileRefMapper);
    if (not memgraphAdj.loadNodes(settlements,components)) {
        throw std::runtime_error("Could not load settlements into graph adjacency");
    }
    DBSCAN dbscan(eps, minPts);
    auto graph = fishnet::graph::GraphFactory::UndirectedGraph<SettlementType>(std::move(memgraphAdj));
    auto result = dbscan.run(graph);
    using OutputShapeType = fishnet::geometry::MultiPolygon<ShapeType>;
    auto outputLayer = fishnet::VectorIO::empty<OutputShapeType>(spatialRef);
    auto idField = outputLayer.addSizeField(Task::FISHNET_ID_FIELD).value_or_throw();
    auto mergeFunction = IDReduceFunction();
    for(auto && cluster : result.clusters){
        auto settlementMultiPolygon = mergeFunction(cluster);
        auto id = settlementMultiPolygon.key();
        fishnet::Feature<OutputShapeType> feature(settlementMultiPolygon.geometry());
        feature.setAttribute(idField, id);
        outputLayer.addFeature(std::move(feature));
    }
    for(auto && noise : result.noise){
        fishnet::Feature<OutputShapeType> feature(OutputShapeType(noise.geometry()));
        feature.setAttribute(idField, size_t(0));
        outputLayer.addFeature(std::move(feature));
    }
    fishnet::VectorIO::overwrite(outputLayer, fishnet::Shapefile("dbscan_output.shp"));
    return 0;
}
