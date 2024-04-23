#include <gtest/gtest.h>
#include "ApproxAreaFilter.hpp"
#include "ProjectedAreaFilter.hpp"
#include <fishnet/WGS84Ellipsoid.hpp>
#include <fishnet/InsideBoundaryFilter.hpp>
#include "ShapeSamples.h"
#include "Testutil.h"
using namespace fishnet;

static double SQUARE_KILOMETER_IN_SQM = 1000000.0;

TEST(FilterTest, ApproxAreaFilter) {
    ApproxAreaFilter squareKilometerFilter {SQUARE_KILOMETER_IN_SQM};
    EXPECT_EQ(squareKilometerFilter.getType(),UnaryFilterType::ApproxAreaFilter);
    auto polygon = SimplePolygonSamples::SQUARE_KILOMETER;
    // for(const auto & seg : polygon.getSegments()) {
    //     std::cout << WGS84Ellipsoid::distance(seg.p(),seg.q()) << std::endl;
    // }
    EXPECT_TRUE(squareKilometerFilter(polygon));
}

TEST(FilterTest, ProjectedAreaFilter) {
    ProjectedAreaFilter squareKilometerFilter {SQUARE_KILOMETER_IN_SQM};
    EXPECT_EQ(squareKilometerFilter.getType(), UnaryFilterType::ProjectedAreaFilter);
    EXPECT_TRUE(squareKilometerFilter(SimplePolygonSamples::SQUARE_KILOMETER));
}

TEST(FilterTest, InsideBoundaryFilter) {
    geometry::ContainedOrInHoleFilter filter;
    testutil::TODO();
}