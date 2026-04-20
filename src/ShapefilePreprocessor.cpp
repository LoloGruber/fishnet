#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <fishnet/Fishnet.hpp>
#include <fishnet/Task.hpp>
#include <fishnet/TaskConfig.hpp>
#include <fishnet/JSONFilterFactory.hpp>
#include <fishnet/CompositePredicate.hpp>

struct ShapefilePreprocessorConfig {
    constexpr static const char * FILTER_KEY = "filters";
    nlohmann::json filterJson;

    ShapefilePreprocessorConfig(const nlohmann::json & configJson){
        if(configJson.contains(FILTER_KEY))
            configJson.at(FILTER_KEY).get_to(this->filterJson);
    }
};

template<fishnet::geometry::GeometryObject G>
class ShapefilePreprocessor : public Task {
private:
    ShapefilePreprocessorConfig config;
    fishnet::Shapefile inputFile;
    fishnet::Shapefile outputFile;

    fishnet::util::AllOfPredicate<G> loadFilters() const {
        fishnet::util::AllOfPredicate<G> predicate;
        for(auto && filter: JSONFilterFactory<G>::getFilters(config.filterJson)){
            predicate.add(std::move(filter));
        }
        return predicate;
    }

    fishnet::util::AllOfPredicate<G,G> loadBinaryFilters() const {
        fishnet::util::AllOfPredicate<G,G> biPredicate;
        for(auto && biFilter: JSONFilterFactory<G>::getBinaryFilters(config.filterJson)){
            biPredicate.add(std::move(biFilter));
        }
        return biPredicate;
    }
public:
    ShapefilePreprocessor(ShapefilePreprocessorConfig config, fishnet::Shapefile input, fishnet::Shapefile output)
        :Task("ShapefilePreprocessor"),config(std::move(config)),inputFile(std::move(input)),outputFile(std::move(output)){}

    void run() {
        auto filter = loadFilters();
        auto binaryFilter = loadBinaryFilters();
        auto input = fishnet::VectorIO::read<G>(this->inputFile);
        auto proj = []( const auto & feature){
            return feature.getGeometry();
        };
        auto filteredFeatures = fishnet::geometry::filter(input.getFeatures(),proj,binaryFilter,filter);
        fishnet::VectorLayer<G> output = fishnet::VectorIO::emptyCopy<G>(input);
        auto idField = output.hasField(Task::FISHNET_ID_FIELD) ? output.getSizeField(Task::FISHNET_ID_FIELD).value_or_throw() : output.addSizeField(Task::FISHNET_ID_FIELD).value_or_throw();
        auto geometryHasher = std::hash<G>();
        for(auto && feature: filteredFeatures){
            feature.setAttribute(idField, fishnet::normalizeToShpFileIntField(geometryHasher(feature.getGeometry())));
            output.addFeature(std::move(feature));
        }
        fishnet::VectorIO::overwrite(output, this->outputFile);
    }
};
constexpr static const char * OUTPUT_SUFFIX = "_filtered.shp";

int main(int argc, char * argv[]){
    using ShapeType = fishnet::geometry::Polygon<double>;
    CLI::App app{"Shapefile Preprocessor"};
    std::string configFilename;
    std::string inputFilename;
    app.add_option("-i,--input",inputFilename,"Input GIS file for the filter step")->required()->check(CLI::ExistingFile);
    app.add_option("-c,--config", configFilename, "Json description of the preprocessing task")->required()->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);
    auto inputFile = fishnet::GISFactory::asShapefile(inputFilename).value_or_throw();
    auto outputFile = fishnet::Shapefile(inputFile.getPath().stem().string() + OUTPUT_SUFFIX);
    ShapefilePreprocessor<ShapeType>(
        ShapefilePreprocessorConfig {nlohmann::json::parse(std::ifstream(configFilename))},
        std::move(inputFile),
        std::move(outputFile)
    ).run();
    return 0;
}