#include <gtest/gtest.h>
#include "ApproxAreaFilter.hpp"
#include "ProjectedAreaFilter.hpp"
#include <fishnet/WGS84Ellipsoid.hpp>
#include "InsidePolygonFilter.hpp"
#include "ShapeSamples.h"
#include "Testutil.h"
using namespace fishnet;

static double SQUARE_KILOMETER_IN_SQM = 1000000.0;

TEST(FilterTest, ApproxAreaFilter) {
    ApproxAreaFilter squareKilometerFilter {SQUARE_KILOMETER_IN_SQM};
    EXPECT_EQ(ApproxAreaFilter::type(),UnaryFilterType::ApproxAreaFilter);
    auto polygon = SimplePolygonSamples::SQUARE_KILOMETER;
    EXPECT_TRUE(squareKilometerFilter(polygon));
}

TEST(FilterTest, ProjectedAreaFilter) {
    // Implementation Deprecated

    // ProjectedAreaFilter squareKilometerFilter {SQUARE_KILOMETER_IN_SQM};
    // EXPECT_EQ(ProjectedAreaFilter::type(), UnaryFilterType::ProjectedAreaFilter);
    // EXPECT_TRUE(squareKilometerFilter(SimplePolygonSamples::SQUARE_KILOMETER));
}

TEST(FilterTest, InsideBoundaryFilter) {
    using namespace fishnet::geometry;
    InsidePolygonFilter filter;
    auto box = SimplePolygonSamples::aaBB({0,0},{4,4});
    auto inside = SimplePolygonSamples::triangle({1,1},{2,2},{1,2});
    auto intersecting = SimplePolygonSamples::aaRhombus({4,2},2);
    EXPECT_FALSE(filter(box,inside));
    EXPECT_FALSE(filter(box,box));
    EXPECT_TRUE(filter(box,intersecting));
}