#include "ContractionTask.h"
#include <fishnet/Polygon.hpp>
#include "DistanceBiPredicate.hpp"



int main(int argc, const char * argv[]){
    using GeometryType = fishnet::geometry::Polygon<double>;
    json desc = json::parse("{\"memgraph-host\":\"localhost\",\"memgraph-port\":7687}");
    ContractionConfig<GeometryType> config {desc};
    config.contractBiPredicates.push_back(DistanceBiPredicate(500.0));
    config.workers = 1;
    fishnet::Shapefile input {"/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl/Punjab_Small_filtered.shp"};
    fishnet::Shapefile output {"/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl/Punjab_Small_merged.shp"};
    ContractionTask<GeometryType> task {std::move(config),output};
    task.addInput(std::move(input));
    task.run();
    return 0;
}