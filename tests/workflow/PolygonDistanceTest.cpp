#include <gtest/gtest.h>
#include "Testutil.h"
#include <fishnet/PathHelper.h>
#include <fishnet/Shapefile.hpp>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/PolygonDistance.hpp>
#include <fishnet/Polygon.hpp>
#include <fishnet/StopWatch.h>
#include <fishnet/GISFactory.hpp>
#include <magic_enum.hpp>

using namespace fishnet;

enum class ExecutionMode{
    BRUTE_FORCE, SWEEP_LINE, DEFAULT
};

template<ExecutionMode ExecMode>
static std::pair<geometry::Vec2DReal,geometry::Vec2DReal> runScenario(fishnet::geometry::IPolygon auto const & lhs, fishnet::geometry::IPolygon auto const & rhs, size_t repetitions, std::string_view filename){
    assert(repetitions > 0 && lhs != rhs);
    std::pair<geometry::Vec2DReal,geometry::Vec2DReal> result;
    util::StopWatch timer;
    for([[maybe_unused]] auto _ : std::views::iota(0UL,repetitions)){
        if constexpr(ExecMode == ExecutionMode::BRUTE_FORCE){
            result = geometry::__impl::closestPointsBruteForce(lhs.getBoundary().getSegments(),rhs.getBoundary().getSegments());
        }else if constexpr(ExecMode == ExecutionMode::SWEEP_LINE){
            result = geometry::__impl::closestPointsSweep(lhs.getBoundary().getSegments(),rhs.getBoundary().getSegments());
        }else if constexpr(ExecMode == ExecutionMode::DEFAULT){
            result = geometry::closestPoints(lhs,rhs);
        }
    }
    double time = timer.stop();
    size_t numberOfSegments = fishnet::util::size(lhs.getBoundary().getSegments())*fishnet::util::size(rhs.getBoundary().getSegments());
    std::cout << filename << "\t\t["<< std::string(magic_enum::enum_name(ExecMode)) << "]: \t"<< time/double(repetitions) << "s\t\tAVG\t\t\t(N x M: "<< numberOfSegments <<")"<< std::endl;
    return result;
}

static void runScenarios(fishnet::geometry::IPolygon auto const & lhs, fishnet::geometry::IPolygon auto const & rhs, size_t repetitions, std::string_view filename){
    auto [lForce,rForce] = runScenario<ExecutionMode::BRUTE_FORCE>(lhs,rhs,repetitions,filename);
    auto [lSweep,rSweep] = runScenario<ExecutionMode::SWEEP_LINE>(lhs,rhs,repetitions,filename);
    auto [l,r] = runScenario<ExecutionMode::DEFAULT>(lhs,rhs,repetitions,filename);
    EXPECT_TRUE(fishnet::math::areEqual(lForce.distance(rForce),lSweep.distance(rSweep)));
    EXPECT_TRUE(fishnet::math::areEqual(lForce.distance(rForce),l.distance(r)));
}



TEST(PolygonDistanceTest, settlementSamples){
    const std::filesystem::path settlementSamplesPath = util::PathHelper::projectDirectory() / std::filesystem::path("data/testing/Settlement_Samples");
    const size_t repetitions = 10;
    const size_t samples = 1;
    auto files = fishnet::GISFactory::getGISFiles(settlementSamplesPath);
    for(const auto & file: files | std::views::take(samples) ){
        auto layer = fishnet::VectorLayer<geometry::Polygon<double>>::read({file});
        auto geometries = layer.getGeometries();
        auto left = geometries.front();
        auto right = geometries.back();
        runScenarios(left,right,repetitions,file.stem().string());
    }
}