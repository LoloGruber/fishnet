#include <gtest/gtest.h>
#include <fishnet/DelaunayTriangulation.hpp>
#include <fishnet/Vec2D.hpp>
#include <fishnet/Triangle.hpp>
#include <vector>

using namespace fishnet::geometry;

class DelaunayTriangulationTest : public ::testing::Test {};

TEST_F(DelaunayTriangulationTest, squareFourPoints) {
    // Square: (0,0), (1,0), (1,1), (0,1)
    std::vector<Vec2D<double>> points = {
        {0, 0}, {1, 0}, {1, 1}, {0, 1}
    };
    DelaunayTriangulation dt(points);

    auto trianglesOpt = dt.getTriangles();
    ASSERT_TRUE(trianglesOpt.has_value());
    EXPECT_EQ(trianglesOpt.value().size(), 2);

    auto edgesOpt = dt.getEdges();
    ASSERT_TRUE(edgesOpt.has_value());
    // 2 triangles * 3 edges = 6 edges, but one diagonal is shared -> 5 unique edges
    EXPECT_EQ(edgesOpt.value().size(), 5);
}

TEST_F(DelaunayTriangulationTest, lessThanThreePoints) {
    std::vector<Vec2D<double>> twoPoints = {{0, 0}, {1, 1}};
    DelaunayTriangulation dt(twoPoints);

    EXPECT_FALSE(dt.getTriangles().has_value());
    EXPECT_FALSE(dt.getEdges().has_value());
}

TEST_F(DelaunayTriangulationTest, triangleThreePoints) {
    std::vector<Vec2D<double>> points = {{0, 0}, {1, 0}, {0, 1}};
    DelaunayTriangulation dt(points);

    auto trianglesOpt = dt.getTriangles();
    ASSERT_TRUE(trianglesOpt.has_value());
    EXPECT_EQ(trianglesOpt.value().size(), 1);
    EXPECT_EQ(trianglesOpt.value()[0].area(), 0.5);

    auto edgesOpt = dt.getEdges();
    ASSERT_TRUE(edgesOpt.has_value());
    EXPECT_EQ(edgesOpt.value().size(), 3);
}

TEST_F(DelaunayTriangulationTest, fivePoints) {
    // Pentagon-ish shape
    std::vector<Vec2D<double>> points = {
        {0, 0}, {2, 0}, {3, 1}, {1, 3}, {-1, 1}
    };
    DelaunayTriangulation dt(points);

    auto trianglesOpt = dt.getTriangles();
    ASSERT_TRUE(trianglesOpt.has_value());
    EXPECT_GT(trianglesOpt.value().size(), 0);

    auto edgesOpt = dt.getEdges();
    ASSERT_TRUE(edgesOpt.has_value());
    EXPECT_GT(edgesOpt.value().size(), 0);
}

TEST_F(DelaunayTriangulationTest, uniqueEdgesNoDuplicates) {
    std::vector<Vec2D<double>> points = {
        {0, 0}, {1, 0}, {0.5, 1}, {0.5, 0.3}
    };
    DelaunayTriangulation dt(points);

    auto edgesOpt = dt.getEdges();
    ASSERT_TRUE(edgesOpt.has_value());
    auto edges = edgesOpt.value();

    // Ensure no duplicate edges using Segment's commutative operator==
    for (size_t i = 0; i < edges.size(); ++i) {
        for (size_t j = i + 1; j < edges.size(); ++j) {
            EXPECT_NE(edges[i], edges[j]) << "Duplicate edge found at indices " << i << " and " << j;
        }
    }
}

TEST_F(DelaunayTriangulationTest, moveSemantics) {
    std::vector<Vec2D<double>> points = {{0, 0}, {1, 0}, {0, 1}};
    DelaunayTriangulation dt1(points);
    ASSERT_TRUE(dt1.getTriangles().has_value());

    DelaunayTriangulation dt2(std::move(dt1));
    ASSERT_TRUE(dt2.getTriangles().has_value());
    EXPECT_EQ(dt2.getTriangles().value().size(), 1);

    std::vector<Vec2D<double>> dummy = {{0, 0}};
    DelaunayTriangulation dt3(dummy);
    dt3 = std::move(dt2);
    ASSERT_TRUE(dt3.getTriangles().has_value());
    EXPECT_EQ(dt3.getTriangles().value().size(), 1);
}
