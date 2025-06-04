#include <gtest/gtest.h>
#include "Testutil.h"
#include <fstream>
#include <fishnet/PathHelper.h>
#include <fishnet/Shapefile.hpp>
#include <fishnet/VectorIO.hpp>
#include <fishnet/PolygonDistance.hpp>
#include <fishnet/Polygon.hpp>
#include <fishnet/StopWatch.h>
#include <fishnet/GISFactory.hpp>
#include <magic_enum.hpp>

using namespace fishnet;

enum class ExecutionMode{
    BRUTE_FORCE, SWEEP_LINE_X, SWEEP_LINE_Y, DEFAULT
};

static inline std::ofstream benchmarkFile = std::ofstream(util::PathHelper::projectDirectory() / std::filesystem::path("doc/benchmarks/tests/polygon-distance-benchmark.csv"));

template<ExecutionMode ExecMode>
static std::pair<geometry::Vec2DReal,geometry::Vec2DReal> runScenario(fishnet::geometry::IPolygon auto const & lhs, fishnet::geometry::IPolygon auto const & rhs, size_t repetitions, std::string_view filename){
    assert(repetitions > 0 && lhs != rhs);
    std::pair<geometry::Vec2DReal,geometry::Vec2DReal> result;
    size_t numberOfSegments = fishnet::util::size(lhs.getBoundary().getSegments())*fishnet::util::size(rhs.getBoundary().getSegments());
    benchmarkFile << filename <<";"<< std::string(magic_enum::enum_name(ExecMode)) << ";";
    benchmarkFile << fishnet::util::size(lhs.getBoundary().getSegments()) <<";"<<fishnet::util::size(rhs.getBoundary().getSegments())<< ";"<<numberOfSegments <<";";
    util::StopWatch timer;

    for([[maybe_unused]] auto _ : std::views::iota(0UL,repetitions)){
        if constexpr(ExecMode == ExecutionMode::BRUTE_FORCE){
            result = geometry::__impl::closestPointsBruteForce(lhs.getBoundary().getSegments(),rhs.getBoundary().getSegments());
        }else if constexpr(ExecMode == ExecutionMode::SWEEP_LINE_X){
            result = geometry::__impl::closestPointsSweep<true>(lhs.getBoundary().getSegments(),rhs.getBoundary().getSegments());
        }else if constexpr(ExecMode == ExecutionMode::SWEEP_LINE_Y){
            result = geometry::__impl::closestPointsSweep<false>(lhs.getBoundary().getSegments(), rhs.getBoundary().getSegments());
        }else if constexpr(ExecMode == ExecutionMode::DEFAULT){
            result = geometry::closestPoints(lhs,rhs);
        }
    }
    double time = timer.stop();

    benchmarkFile << time/double(repetitions) << std::endl;
    return result;
}

static void runScenarios(fishnet::geometry::IPolygon auto const & lhs, fishnet::geometry::IPolygon auto const & rhs, size_t repetitions, std::string_view filename){
    benchmarkFile << "Scenario; Type; N;M; NxM; AVG of "<< repetitions << " in [s]" << std::endl;
    auto [lForce,rForce] = runScenario<ExecutionMode::BRUTE_FORCE>(lhs,rhs,repetitions,filename);
    auto [lSweep,rSweep] = runScenario<ExecutionMode::SWEEP_LINE_X>(lhs,rhs,repetitions,filename);
    auto [lySweep, rySweep] = runScenario<ExecutionMode::SWEEP_LINE_Y>(lhs,rhs,repetitions,filename);
    auto [l,r] = runScenario<ExecutionMode::DEFAULT>(lhs,rhs,repetitions,filename);
    EXPECT_TRUE(fishnet::math::areEqual(lForce.distance(rForce),lSweep.distance(rSweep)));
    EXPECT_TRUE(fishnet::math::areEqual(lForce.distance(rForce),l.distance(r)));
    EXPECT_TRUE(fishnet::math::areEqual(lySweep.distance(rySweep), l.distance(r)));
}



TEST(PolygonDistanceTest, settlementSamples){
    const std::filesystem::path settlementSamplesPath = util::PathHelper::projectDirectory() / std::filesystem::path("data/testing/Settlement_Samples");
    const size_t repetitions = 1;
    const size_t samples = 1; // 100
    auto files = fishnet::GISFactory::getGISFiles(settlementSamplesPath);
    for(const auto & file: files | std::views::take(samples) ){
        auto layer = fishnet::VectorIO::readPolygonLayer({file});
        auto geometries = layer.getGeometries();
        auto left = geometries.front();
        auto right = geometries.back();
        runScenarios(left,right,repetitions,file.stem().string());
    }
}