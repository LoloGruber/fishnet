#include <fishnet/VectorLayer.hpp>
#include <fishnet/PathHelper.h>
#include <cassert>

using namespace fishnet;

int main(int argc, char * argv[]) {
    auto projectDirPath = util::PathHelper::projectDirectory();
    const Shapefile source  {projectDirPath.append("data/WSF/2019/Punjab/Punjab.shp")};
    const auto input = VectorLayer<fishnet::geometry::Polygon<double>>::read(source);
    const int splits = 4;
    
    auto rectangleView = input.getGeometries() | std::views::transform([](const auto & shape){return geometry::Rectangle(shape);});
    double top = std::ranges::max(rectangleView | std::views::transform(&geometry::Rectangle<double>::top));
    double right = std::ranges::max(rectangleView | std::views::transform(&geometry::Rectangle<double>::right));
    double left = std::ranges::min(rectangleView | std::views::transform(&geometry::Rectangle<double>::left));
    double bottom = std::ranges::min(rectangleView | std::views::transform(&geometry::Rectangle<double>::bottom));
    double deltaX = right-left;
    double deltaY = top-bottom;
    std::vector<VectorLayer<fishnet::geometry::Polygon<double>>> outputDatasets;
    outputDatasets.reserve(splits*splits);

    for(int y = 0; y < splits ; y++){
        for(int x = 0; x < splits; x++){
            outputDatasets.push_back(VectorLayer<fishnet::geometry::Polygon<double>>::empty(input.getSpatialReference()));
        }
    }
    for(auto && feature: input.getFeatures()){
        auto centroid = feature.getGeometry().centroid();
        int x = int(((centroid.x - left) / deltaX )* splits);
        int y = int(((centroid.y - bottom) / deltaY) * splits);
        outputDatasets.at(y*splits+x).addFeature(std::move(feature));
    }
    
    for(int y = 0; y < splits ; y++){
        for(int x = 0; x < splits; x++){
            const auto & ds = outputDatasets.at(y*splits+x);
            if(util::size(ds.getFeatures()) > 0) {
                Shapefile dest = source.appendToFilename("_"+ std::to_string(x)+"_"+std::to_string(y));
                ds.overwrite(dest);
            }
        }
    }
    return 0;
}