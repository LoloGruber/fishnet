#include <fishnet/Polygon.hpp>
#include <fishnet/VectorIO.hpp>
#include <fishnet/PathHelper.h>
#include <fishnet/GISFactory.hpp>
#include <fishnet/Rectangle.hpp>
#include <fishnet/Task.hpp>
#include <CLI/CLI.hpp>

using namespace fishnet;
using GeometryType = fishnet::geometry::SimplePolygon<double>;

struct ShapefileSplitterConfig {
    uint32_t depth;
    int xOffset = 0;
    int yOffset = 0;
};

/**
 * @brief Performs a quadtree-based splitting of a shapefile. For example with a depth of 1, the input is split into 4 quadrants
 * 
 */
class ShapefileSplitter: public Task {
public:
    ShapefileSplitter():Task("ShapefileSplitter"){}

    static void operator()(const Shapefile & source, const std::filesystem::path & outputDirectory, const ShapefileSplitterConfig & config ){
        spdlog::set_level(spdlog::level::debug);
        const auto input = VectorIO::read<GeometryType>(source);
        const auto boundingBox = geometry::minimalBoundingBox(input.getGeometries());
        double deltaX = boundingBox.right()-boundingBox.left();
        double deltaY = boundingBox.top()-boundingBox.bottom();
        std::vector<VectorLayer<GeometryType>> outputDatasets;

        size_t chunksPerDimension = static_cast<size_t>(1 << config.depth);
        outputDatasets.reserve(chunksPerDimension*chunksPerDimension);
        spdlog::debug("Depth {}, ChunksPerDimension {}, Total Partitions {}",config.depth,chunksPerDimension,chunksPerDimension*chunksPerDimension);
        for(size_t y = 0; y < chunksPerDimension ; y++){
            for(size_t x = 0; x < chunksPerDimension; x++){
                outputDatasets.push_back(VectorIO::emptyCopy<GeometryType>(input));
            }
        }
        for(auto && feature: input.getFeatures()){
            auto centroid = feature.getGeometry().centroid();
            size_t x = size_t(((centroid.x - boundingBox.left()) / deltaX )* static_cast<double>(chunksPerDimension));
            size_t y = size_t(((centroid.y - boundingBox.bottom()) / deltaY) * static_cast<double>(chunksPerDimension));
            outputDatasets.at(y*chunksPerDimension+x).addFeature(std::move(feature));
        }
        
        for(size_t y = 0; y < chunksPerDimension ; y++){
            for(size_t x = 0; x < chunksPerDimension; x++){
                const auto & ds = outputDatasets.at(y*chunksPerDimension+x);
                if(util::size(ds.getFeatures()) > 0) {
                    Shapefile dest = Shapefile( outputDirectory / fishnet::util::PathHelper::appendToFilename(source.getPath(),"_"+ std::to_string(static_cast<int64_t>(x)+config.xOffset)+"_"+std::to_string(static_cast<int64_t>(y)+config.yOffset)).filename());
                    VectorIO::overwrite(ds, dest);
                }
            }
        }
    }
};

int main(int argc, char * argv[]) {
    CLI::App app  {"Fishnet Shapefile Splitter"};
    std::string inputFilename;
    std::string outputDirectoryName;
    uint32_t depth;
    int xOffset=0;
    int yOffset=0;
    app.add_option("-i,--input",inputFilename,"Path to the input file (.tif or .shp)")->required()->check(CLI::ExistingFile);
    app.add_option("-o,--outputDirectory",outputDirectoryName)->check(CLI::ExistingDirectory)->default_val(std::filesystem::current_path());
    app.add_option("-d,--depth",depth,"Number of vertical/horizontal splits")->required();
    app.add_option("-x",xOffset,"x offset for the tile coordinates of the output files");
    app.add_option("-y",yOffset,"y offset for the tile coordinates of the output files");
    CLI11_PARSE(app,argc,argv);
    ShapefileSplitter()(
        GISFactory::asShapefile(inputFilename).value_or_throw(),
        {outputDirectoryName},
        ShapefileSplitterConfig{.depth=depth,.xOffset=xOffset,.yOffset=yOffset}
    );
    return 0;
}