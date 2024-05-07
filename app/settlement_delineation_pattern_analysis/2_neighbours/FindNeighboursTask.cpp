#include "FindNeighboursTask.h"
#include <fishnet/GISFactory.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>

struct DistanceBiPredicate{
    double maxDistanceInMeters;

    bool operator()(fishnet::geometry::IPolygon auto const & lhs, fishnet::geometry::IPolygon auto const & rhs) const noexcept {
        return lhs.distance(rhs) <= maxDistanceInMeters; // todo use WGS84 DISTANCE
    }
};

int main(int argc, char const *argv[])
{   
    DistanceBiPredicate distancePredicate {3000.0};
    FindNeighboursTask<fishnet::geometry::Polygon<double>> task;
    auto expShp = fishnet::GISFactory::asShapefile("/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/1_filter/cwl/Punjab_Small_filtered.shp");
    task.addShapefile(std::move(getExpectedOrThrowError(expShp)))
        .addNeighbouringPredicate(distancePredicate)
        .setMaxEdgeDistance(distancePredicate.maxDistanceInMeters)
        .setMemgraphParams("localhost",7687);
    task.run();
    return 0;
}
