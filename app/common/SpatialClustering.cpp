#include <CLI/CLI.hpp>
#include <magic_enum.hpp>
#include <fishnet/Fishnet.hpp>
#include <fishnet/TaskConfig.hpp>
#include <fishnet/DistanceFunction.hpp>
#include <fishnet/DistancePredicate.hpp>
#include <fishnet/SettlementShape.hpp>
#include <fishnet/IDReduceFunction.hpp>
#include <fishnet/CachingMemgraphAdjacency.hpp>


enum class ClusterMode {
    DBSCAN,
    BFS
};

struct ClusteringConfig : MemgraphTaskConfig {
    constexpr static const char * CLUSTER_KEY = "clustering";
    constexpr static const char * CLUSTER_MODE_KEY = "mode";
    constexpr static const char * CLUSTER_ARGS_KEY = "args";

    ClusterMode clusterMode;
    json clusterArgs;
    ClusteringConfig(const json & config):MemgraphTaskConfig(config){
        auto clusterConfig = this->jsonDescription.at(CLUSTER_KEY);
        this->clusterMode = magic_enum::enum_cast<ClusterMode>(clusterConfig.at(CLUSTER_MODE_KEY).get<std::string>()).value();
        this->clusterArgs = clusterConfig.at(CLUSTER_ARGS_KEY);
    }

    template<fishnet::graph::Graph G>
    fishnet::ClusterAlgorithm_t<G> getSpatialClusterAlgorithm(DistanceFunction && distanceFunction) const {
        switch(this->clusterMode){
            case ClusterMode::DBSCAN:
                {
                    double eps = clusterArgs.at("distance-threshold").get<double>();
                    size_t minPts = clusterArgs.at("min-cluster-size").get<size_t>(); 
                    return fishnet::DBSCAN<typename G::node_type>(eps, minPts, [&distanceFunction](const typename G::node_type & lhs, const typename G::node_type & rhs){
                        return fishnet::geometry::shapeDistance(lhs,rhs,distanceFunction);
                    });
                }
            case ClusterMode::BFS:
                double distanceThreshold = clusterArgs.at("distance-threshold").get<double>();
                return fishnet::BFSClustering<typename G::node_type>(DistanceBiPredicate(std::move(distanceFunction), distanceThreshold));
        }
        throw std::runtime_error("Unsupported clustering mode");
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

    fishnet::Either<fishnet::VectorLayer<G>,std::string> operator()(const fishnet::Shapefile & shapefile) const {
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
    // Parse cmd arguments
    CLI::App app{"Fishnet Clustering Algorithm"};
    std::vector<std::string> inputfiles;
    std::vector<ComponentReference> components;
    std::string configfile;
    std::string outputStem;
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
    app.add_option("--outputStem", outputStem, "Output filename stem for storing the clustered shapefile");
    CLI11_PARSE(app, argc, argv); 
    
    // Load shapes and settlement graph
    using ShapeType = fishnet::geometry::Polygon<double>;
    using SettlementType = SettlementShape<ShapeType>;
    ClusteringConfig config(nlohmann::json::parse(std::ifstream(configfile)));
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
        return memgraphAdj.getDatabaseConnection().addFileReference(shp.getPath()).value_or_throw("Could not read file reference for shp file:\n"+shp.getPath().string());
    };
    ObservableShapefileReader<ShapeType> reader(onReadStoreSpatialRef);
    auto settlements = SettlementType::read<fishnet::Shapefile>(shapeFiles, reader,fileRefMapper);
    if (not memgraphAdj.loadNodes(settlements,components)) {
        throw std::runtime_error("Could not load settlements into graph adjacency");
    }
    auto graph = fishnet::graph::GraphFactory::UndirectedGraph<SettlementType>(std::move(memgraphAdj));

    // Run clustering
    auto clusterAlgorithm = config.getSpatialClusterAlgorithm<decltype(graph)>(distanceFunctionForSpatialReference(spatialRef));
    auto result = clusterAlgorithm(graph);

    // Store result
    using OutputShapeType = fishnet::geometry::MultiPolygon<ShapeType>;
    auto outputLayer = fishnet::VectorIO::empty<OutputShapeType>(spatialRef);
    auto idField = outputLayer.addSizeField(Task::FISHNET_ID_FIELD).value_or_throw();
    auto mergeFunction = IDReduceFunction();
    for(auto && cluster : result.clusters){
        auto settlementMultiPolygon = mergeFunction(cluster);
        auto id = settlementMultiPolygon.key();
        fishnet::Feature<OutputShapeType> feature(settlementMultiPolygon.geometry());
        feature.setAttribute(idField, size_t(id));
        outputLayer.addFeature(std::move(feature));
    }
    for(auto && noise : result.noise){
        fishnet::Feature<OutputShapeType> feature(OutputShapeType(noise.geometry()));
        feature.setAttribute(idField, size_t(9999999999999));
        outputLayer.addFeature(std::move(feature));
    }
    fishnet::VectorIO::overwrite(outputLayer, fishnet::Shapefile(outputStem + ".shp"));
    return 0;
}

