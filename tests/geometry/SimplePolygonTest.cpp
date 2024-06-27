
#include <gtest/gtest.h>
#include "Testutil.h"
#include <fishnet/SimplePolygon.hpp>
#include <fishnet/GeometryObject.hpp>
#include "ShapeSamples.h"
#include <fishnet/Rectangle.hpp>

using namespace fishnet::geometry;
using namespace testutil;

static_assert(GeometryObject<Ring<double>>);
static_assert(GeometryObject<SimplePolygon<int>>);
static_assert(GeometryObject<Line<long>>);
static_assert(GeometryObject<Vec2D<float>>);

class SimplePolygonTest : public ::testing::Test {
protected:
    void SetUp() override {
        simpleWithComplexBoundary = std::make_unique<SimplePolygon<double>>(LinearRingSamples::COMPLEX_RING);
    }
    std::unique_ptr<SimplePolygon<double>> simpleWithComplexBoundary;

};

TEST_F(SimplePolygonTest , init){
    Ring<int> ring(std::vector<Vec2D<int>>{{0,0},{0,1},{1,1}});
    EXPECT_NO_THROW(SimplePolygon<int> {ring});
    SimplePolygon<int>  fromInitList {std::initializer_list<Vec2D<int>> {Vec2D(0,1),Vec2D(1,1),Vec2D(1,0),Vec2D(0,1)}}; // first and last point equal is allowed
    SimplePolygon<int> fromVectorOfPoints {std::vector<Vec2D<int>>{{Vec2D(0,1),Vec2D(1,1),Vec2D(1,0)}}};
    SimplePolygon<int> fromSegments {std::vector{{
        Segment(Vec2D(0,1),Vec2D(1,1)),
        Segment(Vec2D(1,1),Vec2D(1,0)),
        Segment(Vec2D(1,0),Vec2D(0,1))   
    }}};
    EXPECT_ANY_THROW(SimplePolygon<int> {std::vector<Vec2D<int>> {Vec2D()}}); // too few points
    EXPECT_ANY_THROW(SimplePolygon<int> (std::vector<Segment<int>>({
        Segment(Vec2D(0,1),Vec2D(1,1)),
        Segment(Vec2D(1,1),Vec2D(1,0)),
        Segment(Vec2D(1,0),Vec2D(0,0))  //not closed 
    })));
    EXPECT_ANY_THROW(SimplePolygon<int> ({
        Vec2D(0,0), Vec2D(0,1),Vec2D(1,0),Vec2D(1,1),Vec2D(0,0) // self intersections
    }));
}

TEST_F(SimplePolygonTest, getBoundary) {
    auto boundary = simpleWithComplexBoundary->getBoundary();
    EXPECT_TYPE<Ring<double>>(boundary);
    EXPECT_EQ(boundary,LinearRingSamples::COMPLEX_RING);
}

TEST_F(SimplePolygonTest, getHoles) {
    EXPECT_EMPTY(simpleWithComplexBoundary->getHoles());
}

TEST_F(SimplePolygonTest, isInHole){
    EXPECT_FALSE(simpleWithComplexBoundary->isInHole(*simpleWithComplexBoundary));
}

TEST_F(SimplePolygonTest, containsPoint) {
    EXPECT_TRUE(simpleWithComplexBoundary->contains(Vec2D(0,0)));
    EXPECT_TRUE(SimplePolygon<double> {LinearRingSamples::triangle(Vec2D(0,0),Vec2D(1,1),Vec2D(1,0))}.contains(Vec2D(1,0)));
    EXPECT_TRUE(SimplePolygon<double> {LinearRingSamples::aaBB(Vec2D(0,0),Vec2D(2,2))}.contains(Vec2D(1,1)));
}

TEST_F(SimplePolygonTest, intersects) {
    EXPECT_TRUE(simpleWithComplexBoundary->intersects(Ray<int>::up(Vec2D(1,2))));
    EXPECT_FALSE(simpleWithComplexBoundary->intersects(Line<int>::horizontalLine(5)));
}

TEST_F(SimplePolygonTest, intersections) {
    std::vector<Vec2D<double>> expected {
        Vec2D(2,2),Vec2D(4,2)
    };
    EXPECT_CONTAINS_ALL(simpleWithComplexBoundary->intersections(Ray<int>::right(Vec2D(0.5,2))),expected);
    
    expected = {
        Vec2D(-1,-0.75),Vec2D(-1,0),Vec2D(-1,1 + 1.0/3.0), Vec2D(-1,3+ 1.0/3.0)
    };
    EXPECT_CONTAINS_ALL(simpleWithComplexBoundary->intersections(Line<int>::verticalLine(-1)),expected);
    EXPECT_SIZE(simpleWithComplexBoundary->intersections(Line<int>::verticalLine(-1)),4);
    EXPECT_EMPTY(simpleWithComplexBoundary->intersections(Segment<int>(Vec2D(0,2),Vec2D(1,1))));
}

TEST_F(SimplePolygonTest, containsPolygon) {
    SimplePolygon<double> triangleInside = LinearRingSamples::triangle({1,2},{0.5,2},{0,3});
    EXPECT_TRUE(simpleWithComplexBoundary->contains(triangleInside));
    SimplePolygon<double> crossingBoundary = LinearRingSamples::aaBB({0,4},{1,3});
    EXPECT_FALSE(simpleWithComplexBoundary->contains(crossingBoundary));
}

TEST_F(SimplePolygonTest, equality) {
    EXPECT_EQ(*simpleWithComplexBoundary,*simpleWithComplexBoundary);
    SimplePolygon<double> t1 = LinearRingSamples::triangle({0,0},{0,1},{1,1});
    SimplePolygon<double> t2 = LinearRingSamples::triangle({1,1},{0,1},{0,0});
    EXPECT_EQ(t1,t2);
    EXPECT_NE(*simpleWithComplexBoundary,t1);
}

TEST_F(SimplePolygonTest, rectangleOverlap){
    auto r = Rectangle(Vec2D(0,1),Vec2D(1,0));
    auto intersects = Rectangle(Vec2D(0.5,0.5),Vec2DStd(10,0));
    auto touches = Rectangle(Vec2D(1,2),Vec2D(2,1));
    auto noOverlap = Rectangle(Vec2D(2,1),Vec2D(3,0));
    EXPECT_TRUE(r.overlap(intersects));
    EXPECT_TRUE(r.overlap(touches));
    EXPECT_FALSE(r.overlap(noOverlap));
    EXPECT_FALSE(r.overlap(Rectangle(Vec2D(0.5,2),Vec2D(1.5,1.5))));
}

