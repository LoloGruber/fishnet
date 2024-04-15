#include <gtest/gtest.h>
#include <fishnet/MultiPolygon.hpp>
#include <fishnet/Polygon.hpp>
#include "Testutil.h"
#include "ShapeSamples.h"

using namespace testutil;
using namespace fishnet::geometry;
using namespace fishnet::math;

static_assert(IMultiPolygon<MultiPolygon<Polygon<double>>>);
static_assert(ShapeGeometry<MultiPolygon<SimplePolygon<double>>>);
class MultiPolygonTest : public ::testing::Test {
protected:
    void SetUp() override {
        multiPolygon = MultiPolygon<SimplePolygon<double>>(complex);
        multiPolygon.addPolygon(convex);
        multiPolygon.addPolygon(box);
    }

    std::vector<Vec2D<double>> points = {{3,4},{4,5},{5,4},{5,3},{4,3}};
    SimplePolygon<double> complex = SimplePolygonSamples::COMPLEX_BOUNDARY;
    SimplePolygon<double> convex {points};
    SimplePolygon<double> box = SimplePolygonSamples::aaBB({-3,1},{-2,-0.5});

    MultiPolygon<SimplePolygon<double>> multiPolygon {complex}; 

};

TEST_F(MultiPolygonTest, init) {
    auto s1 = SimplePolygonSamples::aaBB({0,0},{1,1});
    auto s2 = SimplePolygonSamples::aaBB({2,2},{4,4});
    auto multi = MultiPolygon(s1);
    multi.addPolygon(s2);
    EXPECT_UNSORTED_RANGE_EQ(multi.getPolygons(),std::vector<SimplePolygon<double>>{s1,s2});
    EXPECT_NO_FATAL_FAILURE(MultiPolygon(s1,s2));
    std::vector<SimplePolygon<double>> crossingPolygons = {
        SimplePolygonSamples::aaBB({2,2},{0,0}),
        SimplePolygonSamples::aaRhombus({2,2},1)
    };
    EXPECT_ANY_THROW(MultiPolygon<SimplePolygon<double>> {crossingPolygons});
    EXPECT_ANY_THROW(MultiPolygon<SimplePolygon<double>> (s1,s2,s1));
    EXPECT_ANY_THROW(MultiPolygon<SimplePolygon<double>> (s1,crossingPolygons.at(0)));
    EXPECT_ANY_THROW(MultiPolygon<SimplePolygon<double>> (s1,SimplePolygon(s1.getBoundary().aaBB())));
}

TEST_F(MultiPolygonTest, getPolygons) {
    EXPECT_UNSORTED_RANGE_EQ(multiPolygon.getPolygons(),std::initializer_list<Polygon<double>>{complex,convex,box});
    EXPECT_SIZE(MultiPolygon<Polygon<double>>(Polygon<double>(convex)).getPolygons(),1);
}

TEST_F(MultiPolygonTest, addPolygon) {
    SimplePolygon<double> another = SimplePolygonSamples::aaBB({-3,5},{-1,4});
    EXPECT_TRUE(multiPolygon.addPolygon(another));
    SimplePolygon<double> crossingMulti = SimplePolygonSamples::aaRhombus({0,-1},2);
    EXPECT_FALSE(multiPolygon.addPolygon(crossingMulti));
    EXPECT_SIZE(multiPolygon.getPolygons(),4);
    EXPECT_FALSE(multiPolygon.addPolygon(complex));
    EXPECT_SIZE(multiPolygon.getPolygons(),4);
    SimplePolygon<double> completelyInside = SimplePolygonSamples::triangle({0,2},{1,3},{1,2});
    EXPECT_FALSE(multiPolygon.addPolygon(completelyInside));
    EXPECT_SIZE(multiPolygon.getPolygons(),4);
    SimplePolygon<double> touching = SimplePolygonSamples::aaBB({5,4},{6,3});
    EXPECT_TRUE(multiPolygon.addPolygon(touching));
    EXPECT_SIZE(multiPolygon.getPolygons(),5);
}

TEST_F(MultiPolygonTest, removePolygon){
    EXPECT_TRUE(multiPolygon.removePolygon(complex));
    EXPECT_SIZE(multiPolygon.getPolygons(),2);
    EXPECT_NOT_CONTAINS(multiPolygon.getPolygons(),complex);
    EXPECT_FALSE(multiPolygon.removePolygon(SimplePolygonSamples::aaBB({0,0},{1,1})));
    EXPECT_SIZE(multiPolygon.getPolygons(),2);
}

TEST_F(MultiPolygonTest, area){
    EXPECT_DOUBLE_EQ(multiPolygon.area(),complex.area()+convex.area()+box.area());
    EXPECT_TRUE(multiPolygon.removePolygon(box));
    EXPECT_DOUBLE_EQ(multiPolygon.area(), complex.area()+convex.area());
    MultiPolygon<SimplePolygon<double>> simple {SimplePolygonSamples::aaBB({0,0},{1,1}),SimplePolygonSamples::aaBB({3,4},{4,2})};
    EXPECT_DOUBLE_EQ(simple.area(),3.0);
}

TEST_F(MultiPolygonTest,centroid) {
    auto expectedCentroid = complex.centroid()*(complex.area()/multiPolygon.area())
        + convex.centroid() * (convex.area()/multiPolygon.area())
        + box.centroid() * (box.area() / multiPolygon.area());
    EXPECT_EQ(multiPolygon.centroid(), expectedCentroid);
    MultiPolygon<SimplePolygon<double>> twoBoxesStacked {SimplePolygonSamples::aaBB({0,0},{1,1}),SimplePolygonSamples::aaBB({0,1},{1,2})};
    EXPECT_EQ(twoBoxesStacked.centroid(), Vec2D(0.5,1)); 
}

TEST_F(MultiPolygonTest, containsPoint) {
    EXPECT_TRUE(multiPolygon.contains(Vec2D(0,0)));
    EXPECT_TRUE(multiPolygon.contains(Vec2D(-2,-1)));
    EXPECT_TRUE(multiPolygon.contains(Vec2D(-2.5,0)));
    EXPECT_TRUE(multiPolygon.contains(Vec2D(-2,1)));
    EXPECT_TRUE(multiPolygon.contains(Vec2D(4,4)));
    EXPECT_FALSE(multiPolygon.contains(Vec2D(-1,1)));
    EXPECT_FALSE(multiPolygon.contains(Vec2D(3,3)));
    MultiPolygon<SimplePolygon<double>> touchingMultiPolygon = {
        SimplePolygonSamples::aaRhombus({0,0},1),
        SimplePolygonSamples::aaRhombus({2,0},1)
    };
    EXPECT_TRUE(touchingMultiPolygon.contains(Vec2D{1,0}));
    EXPECT_FALSE(touchingMultiPolygon.contains(Vec2D(1,1)));
}

TEST_F(MultiPolygonTest, containsSegment) {
    EXPECT_TRUE(multiPolygon.contains(Segment<double>({-2,-1},{3,1})));
    EXPECT_TRUE(multiPolygon.contains(Segment<double>({-3,1},{-2.5,0.25})));
    EXPECT_FALSE(multiPolygon.contains(Segment<double>({5,4},{0,2})));
    EXPECT_FALSE(multiPolygon.contains(Segment<double>({100,100},{101,100})));
}

TEST_F(MultiPolygonTest, isInside) {
    EXPECT_TRUE(multiPolygon.isInside(Vec2D(0,0)));
    EXPECT_TRUE(multiPolygon.isInside(Vec2D(-2.5,0)));
    EXPECT_TRUE(multiPolygon.isInside(Vec2D(4,4)));
    EXPECT_FALSE(multiPolygon.isInside(Vec2D(0,1)));
    EXPECT_FALSE(multiPolygon.isInside(Vec2D(7,5)));
    MultiPolygon<Polygon<double>> multiWithHoles = {
        Polygon<double>(SimplePolygonSamples::COMPLEX_BOUNDARY,{SimplePolygonSamples::aaRhombus({1,2},1)}),
        Polygon<double>(SimplePolygonSamples::triangle({4,2},{5,3},{2,2}))
    };
    EXPECT_TRUE(multiWithHoles.isInside(Vec2D(-2,2)));
    EXPECT_FALSE(multiWithHoles.isInside(Vec2D(1,2)));
    EXPECT_FALSE(multiWithHoles.isInside(Vec2D(1,2.9999)));
    EXPECT_TRUE(multiWithHoles.isInside(Vec2D(1,3))); // is on boundary of hole, but inside of outer boundary -> TRUE 
    EXPECT_FALSE(multiWithHoles.isInside(Vec2D(4,2)));
}

TEST_F(MultiPolygonTest, isOnBoundary) {
    EXPECT_TRUE(multiPolygon.isOnBoundary(Vec2D(2,0)));
    EXPECT_TRUE(multiPolygon.isOnBoundary(Vec2D(-2,0)));
    EXPECT_FALSE(multiPolygon.isOnBoundary(Vec2D(0,0)));
    EXPECT_FALSE(multiPolygon.isOnBoundary(Vec2D(4,4)));
    EXPECT_FALSE(multiPolygon.isOnBoundary(Vec2D(3,-1)));
}

TEST_F(MultiPolygonTest, isOutside) {
    EXPECT_TRUE(multiPolygon.isOutside(Vec2D(3,-1)));
    EXPECT_FALSE(multiPolygon.isOutside(Vec2D(4,3)));
    EXPECT_FALSE(multiPolygon.isOutside(Vec2D(0,0)));
}

TEST_F(MultiPolygonTest, intersects) {
    EXPECT_FALSE(multiPolygon.intersects(Segment<double>(Vec2D(-2.8,0),Vec2D(-2.1,0))));
    EXPECT_TRUE(multiPolygon.intersects(xAxis));
    EXPECT_TRUE(multiPolygon.intersects(Ray<double>::up({4,3.5})));
    EXPECT_FALSE(multiPolygon.intersects(Segment(Vec2D(5,2),Vec2D(2,4))));
}

TEST_F(MultiPolygonTest, intersections) {
    std::vector<Vec2D<DEFAULT_NUMERIC>> expected {
        {5,3},{4,3},{2,3},{-1.5,3}
    };
    EXPECT_CONTAINS_ALL(multiPolygon.intersections(Line<int>::horizontalLine(3)),expected);
    EXPECT_EMPTY(multiPolygon.intersections(Ray<int>::down(Vec2D(4,1))));
}

TEST_F(MultiPolygonTest, containsPolygon) {
    EXPECT_TRUE(multiPolygon.contains(box));
    EXPECT_TRUE(multiPolygon.contains(complex));
    EXPECT_FALSE(multiPolygon.contains(complex.aaBB()));
}

TEST_F(MultiPolygonTest, isInHolePolygon) {
    EXPECT_FALSE(multiPolygon.isInHole(complex));
    EXPECT_FALSE(multiPolygon.isInHole(convex));
    MultiPolygon<Polygon<double>> multiWithHoles {Polygon<double>(complex,{SimplePolygonSamples::aaBB({0,3},{1,2})})};
    EXPECT_TRUE(multiWithHoles.isInHole(SimplePolygonSamples::aaBB({0.5,2.5},{0.7,2.2})));
    EXPECT_FALSE(multiWithHoles.isInHole(SimplePolygonSamples::triangle({0,3},{1,2},{2,2})));
}

TEST_F(MultiPolygonTest, crossesPolygon) {
    EXPECT_FALSE(multiPolygon.crosses(box));
    EXPECT_TRUE(multiPolygon.crosses(SimplePolygonSamples::aaBB(Vec2D(2,0),Vec2D(4,1))));
}

TEST_F(MultiPolygonTest, touchesPolygon) {
    EXPECT_FALSE(multiPolygon.touches(box));
    EXPECT_TRUE(multiPolygon.touches(SimplePolygonSamples::triangle(Vec2D(0,4),Vec2D(2,5),Vec2D(3,4))));
}

TEST_F(MultiPolygonTest, distancePolygon) {
    EXPECT_EQ(multiPolygon.distance(SimplePolygonSamples::aaBB(Vec2D(6,3),Vec2D(7,0))),1.0);
}

TEST_F(MultiPolygonTest, distanceMultiPolygon) {
    MultiPolygon<Polygon<double>> other = {
         Polygon<double>(SimplePolygonSamples::aaRhombus(Vec2D(-2,-3),1)),
         Polygon<double>(SimplePolygonSamples::aaBB({-5,1},{-3.5,0}))
    };
    EXPECT_EQ(multiPolygon.distance(other),0.5);
}

TEST_F(MultiPolygonTest, equality) {
    EXPECT_EQ(multiPolygon,multiPolygon);
    auto copy = multiPolygon;
    copy.removePolygon(convex);
    EXPECT_NE(multiPolygon,copy);
    EXPECT_NE(multiPolygon,MultiPolygon<Polygon<double>>(Polygon<double>(LinearRingSamples::aaBB(Vec2D(0,0),Vec2D(1,1)))));
    MultiPolygon<SimplePolygon<double>> differentOrder {box,convex,complex};
    EXPECT_EQ(multiPolygon,differentOrder);
}

TEST_F(MultiPolygonTest, aaBB) {
    EXPECT_EQ(multiPolygon.aaBB(),LinearRingSamples::aaBB(Vec2D(-3,5),Vec2D(5,-1)));
}

TEST_F(MultiPolygonTest, toString){
    // std::cout << multiPolygon.toString() << std::endl;
    EXPECT_EQ(multiPolygon.toString(),"{"+complex.toString()+"|"+convex.toString()+"|"+box.toString()+"}");
}



