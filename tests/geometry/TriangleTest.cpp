#include <gtest/gtest.h>
#include <fishnet/Triangle.hpp>
#include <fishnet/Vec2D.hpp>
#include "ShapeSamples.h"

using namespace fishnet::geometry;

class TriangleTest : public ::testing::Test {
protected:
    Vec2D<double> a{0, 0};
    Vec2D<double> b{4, 0};
    Vec2D<double> c{0, 3};
    Triangle<double> tri{a, b, c};
};

TEST_F(TriangleTest, init){
    Vec2D<double> p1{1, 1};
    Vec2D<double> p2{2, 1};
    Vec2D<double> p3{1, 2};
    EXPECT_NO_FATAL_FAILURE(auto triangle = Triangle<double>(p1, p2, p3));
}

TEST_F(TriangleTest, getPoints) {
    auto pts = tri.getPoints();
    EXPECT_EQ(pts[0], a);
    EXPECT_EQ(pts[1], b);
    EXPECT_EQ(pts[2], c);
}

TEST_F(TriangleTest, area) {
    // Right triangle with legs 4 and 3 -> area = 6
    EXPECT_DOUBLE_EQ(tri.area(), 6.0);
}

TEST_F(TriangleTest, centroid) {
    // Centroid of (0,0), (4,0), (0,3) = (4/3, 1)
    auto cent = tri.centroid();
    EXPECT_NEAR(cent.x, 4.0 / 3.0, 1e-10);
    EXPECT_NEAR(cent.y, 1.0, 1e-10);
}

TEST_F(TriangleTest, equality) {
    Triangle<double> same{a, b, c};
    EXPECT_EQ(tri, same);
}

TEST_F(TriangleTest, inequality) {
    Triangle<double> different{a, b, Vec2D<double>(0, 4)};
    EXPECT_NE(tri, different);
}

TEST_F(TriangleTest, shapeConcepts) {
    // Contains point
    EXPECT_TRUE(tri.contains(Vec2D<double>(1, 1)));
    EXPECT_FALSE(tri.contains(Vec2D<double>(10, 10)));

    // Boundary / inside / outside
    EXPECT_TRUE(tri.isOnBoundary(a));
    EXPECT_TRUE(tri.isInside(Vec2D<double>(1, 1)));
    EXPECT_TRUE(tri.isOutside(Vec2D<double>(10, 10)));
}

TEST_F(TriangleTest, toString) {
    auto s = tri.toString();
    EXPECT_NE(s.find("Triangle"), std::string::npos);
    EXPECT_NE(s.find("0.000"), std::string::npos);
    EXPECT_NE(s.find("4.000"), std::string::npos);
    EXPECT_NE(s.find("3.000"), std::string::npos);
}
