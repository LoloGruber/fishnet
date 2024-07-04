#include <gtest/gtest.h>
#include "Testutil.h"
#include <fishnet/PathHelper.h>
#include <fishnet/Shapefile.hpp>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/PolygonDistance.hpp>
#include <fishnet/Polygon.hpp>
#include <fishnet/StopWatch.h>
#include <magic_enum.hpp>

using namespace fishnet;

enum class ExecutionMode{
    BRUTE_FORCE, SWEEP
};

template<ExecutionMode ExecMode>
static std::vector<std::pair<geometry::Vec2DReal,geometry::Vec2DReal>> runScenario(geometry::PolygonRange auto const & polygons){
    std::vector<std::pair<geometry::Vec2DReal,geometry::Vec2DReal>> result;
    util::StopWatch timer {std::string(magic_enum::enum_name(ExecMode))};
    for(const auto & lhs : polygons){
        for(const auto & rhs : polygons){
            if(lhs != rhs){
                if constexpr(ExecMode == ExecutionMode::BRUTE_FORCE){
                    result.push_back(geometry::__impl::closestPointsBruteForce(lhs.getBoundary(),rhs.getBoundary()));
                }else if constexpr(ExecMode == ExecutionMode::SWEEP){
                    result.push_back(geometry::closestPoints(lhs.getBoundary(),rhs.getBoundary()));
                }
            }
        }
    }
    return result;
}

TEST(PolygonDistanceTest, small){
    Shapefile path {util::PathHelper::projectDirectory() / std::filesystem::path("data/testing/Large_Settlements/Bolivia.shp")};
    auto vectorLayer = VectorLayer<geometry::Polygon<double>>::read(path);
    auto polygons = vectorLayer.getGeometries();
    runScenario<ExecutionMode::BRUTE_FORCE>(polygons);
    runScenario<ExecutionMode::SWEEP>(polygons);
}