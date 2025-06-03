#include <fishnet/Polygon.hpp>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/PathHelper.h>
#include <fishnet/GISFactory.hpp>
#include <fishnet/BoundingBoxPolygon.hpp>
#include <fishnet/StopWatch.h>
#include <cassert>
#include <CLI/CLI.hpp>

using namespace fishnet;

int main(int argc, char * argv[]) {
    CLI::App app  {"Fishnet Shapefile Splitter"};
    fishnet::util::StopWatch splitTask {"Split Task"};
    std::string inputFilename;//="/home/lolo/Documents/fishnet/data/WSF/2019/Casablanca/Small/WesternCasablanca.tif";
    std::string outputDirectoryName;//="/home/lolo/Documents/fishnet/data/WSF/2019/Casablanca/Small/";
    uint32_t splits;//=3;
    int xOffset=0;
    int yOffset=0;
    app.add_option("-i,--input",inputFilename,"Path to the input file (.tif or .shp)")->required()->check(CLI::ExistingFile);
    app.add_option("-o,--outputDirectory",outputDirectoryName)->required()->check(CLI::ExistingDirectory);
    app.add_option("-s",splits,"Number of vertical/horizontal splits")->required();
    app.add_option("-x",xOffset,"x offset for the tile coordinates of the output files");
    app.add_option("-y",yOffset,"y offset for the tile coordinates of the output files");
    CLI11_PARSE(app,argc,argv);
    const auto expSource = GISFactory::asShapefile(inputFilename);
    if(not expSource)
        throw std::runtime_error(expSource.error());
    const Shapefile & source = expSource.value();
    const auto input = VectorLayer<fishnet::geometry::Polygon<double>>::read(source);
    const std::filesystem::path outputDir {outputDirectoryName};
    const auto boundingBox = geometry::minimalBoundingBox(input.getGeometries());
    double deltaX = boundingBox.right()-boundingBox.left();
    double deltaY = boundingBox.top()-boundingBox.bottom();
    std::vector<VectorLayer<fishnet::geometry::Polygon<double>>> outputDatasets;
    auto pieces = splits+1;
    outputDatasets.reserve(pieces*pieces);

    for(uint32_t y = 0; y < pieces ; y++){
        for(uint32_t x = 0; x < pieces; x++){
            outputDatasets.push_back(VectorLayer<fishnet::geometry::Polygon<double>>::empty(input.getSpatialReference()));
        }
    }
    for(auto && feature: input.getFeatures()){
        auto centroid = feature.getGeometry().centroid();
        int x = int(((centroid.x - boundingBox.left()) / deltaX )* pieces);
        int y = int(((centroid.y - boundingBox.bottom()) / deltaY) * pieces);
        outputDatasets.at(y*pieces+x).addFeature(std::move(feature));
    }
    
    for(uint32_t y = 0; y < pieces ; y++){
        for(uint32_t x = 0; x < pieces; x++){
            const auto & ds = outputDatasets.at(y*pieces+x);
            if(util::size(ds.getFeatures()) > 0) {
                Shapefile dest = outputDir / fishnet::util::PathHelper::appendToFilename(source.getPath(),"_"+ std::to_string(x+xOffset)+"_"+std::to_string(y+yOffset)).filename();
                ds.overwrite(dest);
            }
        }
    }
    std::cout << "{duration[s]:"<< splitTask.stop() << "}" << std::endl;
    return 0;
}