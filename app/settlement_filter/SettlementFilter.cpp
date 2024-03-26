#include "VectorLayer.hpp"
#include "Polygon.hpp"
#include "PolygonFilter.hpp"
#include "StopWatch.h"
#include "FunctionalConcepts.hpp"

using namespace fishnet;

int main(int argc, char * argv[]) {
//    VectorLayer<geometry::Polygon<double>> inputLayer = VectorLayer<geometry::Polygon<double>>::read({"/home/lolo/Documents/fishnet/2022-mp-lorenz-gruber/data/WSF/WSF3D/Punjab-India/Punjab_shp.shp"}); //todo filter and store small test set for testing
//    size_t limit = 123;
//    StopWatch filter {"Filter"};
//    auto result = geometry::filter(inputLayer.getGeometries(),geometry::ContainedOrInHoleFilter());
//    result = geometry::filter(result,util::LimitPredicate(limit));
//    filter.stopAndPrint();
//    VectorLayer<geometry::Polygon<double>> outputLayer = VectorLayer<geometry::Polygon<double>>::overwrite({"/home/lolo/Documents/fishnet/2022-mp-lorenz-gruber/data/output/small_dataset/Punjab_Small.shp"},inputLayer.getSpatialReference());
//    outputLayer.addAllGeometry(result);
//    outputLayer.write();
    return 0;
}