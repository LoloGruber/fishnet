#include <gtest/gtest.h>
#include <fishnet/Ring.hpp>
#include "Testutil.h"

using namespace fishnet::geometry;
using namespace testutil;

class RingTest: public ::testing::Test{
protected:
    void SetUp() override {
        points = std::vector<Vec2D<double>>();
        points.emplace_back(Vec2D<double>(0,4));
        points.emplace_back(Vec2D<double>(2,3));
        points.emplace_back(Vec2D<double>(2,2));
        points.emplace_back(Vec2D<double>(4,2));
        points.emplace_back(Vec2D<double>(2,0));
        points.emplace_back(Vec2D<double>(-2,-1));
        points.emplace_back(Vec2D<double>(0,1));
        points.emplace_back(Vec2D<double>(-3,2));
        ring = std::make_unique<Ring<double>>(points);

        segments = std::vector<Segment<double>>();
        segments.emplace_back(Segment<double>(Vec2D<double>(3,4),Vec2D<double>(4,5)));
        segments.emplace_back(Segment<double>(Vec2D<double>(4,5),Vec2D<double>(5,4)));
        segments.emplace_back(Segment<double>(Vec2D<double>(5,4),Vec2D<double>(5,3)));
        segments.emplace_back(Segment<double>(Vec2D<double>(5,3),Vec2D<double>(4,3)));
        segments.emplace_back(Segment<double>(Vec2D<double>(4,3),Vec2D<double>(3,4)));
        convex = std::make_unique<Ring<double>>(segments);

        square = std::make_unique<Ring<int>>(std::vector<Vec2D<int>>{Vec2D(0,0),Vec2D(0,1),Vec2D(1,1),Vec2D(1,0)});
    }

    std::vector<Segment<double>> segments;
    std::vector<Vec2D<double>> points;
    // using pointers, since Ring is not standard constructable
    std::unique_ptr<Ring<double>> ring;
    std::unique_ptr<Ring<double>> convex;
    std::unique_ptr<Ring<int>> square;
};

TEST_F(RingTest, init){
    std::vector<Segment<int>> segments {
        Segment(Vec2D(0,0),Vec2D(0,1)),
        Segment(Vec2D(0,1),Vec2D(1,1)),
        Segment(Vec2D(1,0),Vec2D(1,1)), // segment connects with neighbours but p and q must be flipped when calling getPoints()
        Segment(Vec2D(1,0),Vec2D(0,0))
    };
    Ring<int> r {segments};
    EXPECT_RANGE_EQ(r.getSegments(),segments);
    std::vector<Vec2D<int>> expectedPoints {
        Vec2D(0,0), Vec2D(0,1),Vec2D(1,1),Vec2D(1,0)
    };
    EXPECT_RANGE_EQ(r.getPoints(), expectedPoints);
    std::vector<Vec2D<int>> notEnoughPoints {Vec2D<int>(0,0),Vec2D<int>(1,1)};
    EXPECT_ANY_THROW(Ring<int>(std::vector<Vec2D<int>>()));
    EXPECT_ANY_THROW(Ring<int>{notEnoughPoints});
    std::vector<Segment<int>> intersecting {
        Segment(Vec2D(0,0),Vec2D(1,1)),
        Segment(Vec2D(1,1),Vec2D(2,-1)),
        Segment(Vec2D(2,-1),Vec2D(2,1)),
        Segment(Vec2D(2,1),Vec2D(0,0))
    };
    EXPECT_ANY_THROW(Ring<int>{intersecting});
    std::vector<Segment<int>> notClosed {
        Segment(Vec2D(0,0),Vec2D(1,1)),
        Segment(Vec2D(1,1),Vec2D(2,1)),
        Segment(Vec2D(2,1),Vec2D(0,-2)),
        Segment(Vec2D(0,-2),Vec2D(-1,0))
    };
    EXPECT_ANY_THROW(Ring<int>{notClosed});

    std::vector<Segment<int>> flippedSegments {
        Segment(Vec2D(0,0),Vec2D(0,1)),
        Segment(Vec2D(1,1),Vec2D(0,1)),
        Segment(Vec2D(1,0),Vec2D(1,1)), 
        Segment(Vec2D(0,0),Vec2D(1,0))
    };
    Ring<int> valid {flippedSegments};
    std::vector<Segment<int>> expectedSegments {
        Segment(Vec2D(0,0),Vec2D(0,1)),
        Segment(Vec2D(0,1),Vec2D(1,1)),
        Segment(Vec2D(1,1),Vec2D(1,0)), 
        Segment(Vec2D(1,0),Vec2D(0,0))
    };
    EXPECT_RANGE_EQ(valid.getPoints(),expectedPoints);
    EXPECT_RANGE_EQ(valid.getSegments(),expectedSegments);
}

TEST_F(RingTest, castToDouble) {
    auto squareAsDoubleRing = Ring<double>(*square);
    EXPECT_TYPE<Ring<double>>(squareAsDoubleRing);
}

TEST_F(RingTest, getter){
    EXPECT_RANGE_EQ(ring->getPoints(),points);
    std::vector<Segment<double>> expectedRingSegments {
        Segment<double>(Vec2D<double>(0,4),Vec2D<double>(2,3)),
        Segment<double>(Vec2D<double>(2,3),Vec2D<double>(2,2)),
        Segment<double>(Vec2D<double>(2,2),Vec2D<double>(4,2)),
        Segment<double>(Vec2D<double>(4,2),Vec2D<double>(2,0)),
        Segment<double>(Vec2D<double>(2,0),Vec2D<double>(-2,-1)),
        Segment<double>(Vec2D<double>(-2,-1),Vec2D<double>(0,1)),
        Segment<double>(Vec2D<double>(0,1),Vec2D<double>(-3,2)),
        Segment<double>(Vec2D<double>(-3,2),Vec2D<double>(0,4))
    };
    EXPECT_RANGE_EQ(ring->getSegments(),expectedRingSegments);

    EXPECT_RANGE_EQ(convex->getSegments(),segments);
    std::vector<Vec2D<double>> expectedPointsConvex {
        Vec2D(3,4),
        Vec2D(4,5),
        Vec2D(5,4),
        Vec2D(5,3),
        Vec2D(4,3),
    };
    EXPECT_RANGE_EQ(convex->getPoints(),expectedPointsConvex);
}

TEST_F(RingTest, aaBB){
    EXPECT_EQ(ring->aaBB(),Ring<double>({
        Vec2D<double>(-3,4),
        Vec2D<double>(4,4),
        Vec2D<double>(4,-1),
        Vec2D<double>(-3,-1)
    }));
    EXPECT_EQ(square->aaBB(),*square);
}

TEST_F(RingTest, containsPoint){
    for(const auto & p: points){
        EXPECT_TRUE(ring->contains(p));
    }
    EXPECT_TRUE(ring->contains(Vec2D(0.5,2)));
    EXPECT_TRUE(ring->contains(Vec2D(0,0)));
    EXPECT_FALSE(ring->contains(Vec2D(-2,0)));
    EXPECT_FALSE(ring->contains(Vec2D(0,-0.6)));
    EXPECT_FALSE(ring->contains(Vec2D(5,2)));
    auto pointsOfConvex = convex->getPoints();
    for(const auto & p: pointsOfConvex) {
        EXPECT_TRUE(convex->contains(p));
    }
    EXPECT_TRUE(convex->contains(Vec2D(4,4)));
    EXPECT_FALSE(convex->contains(Vec2D(3.5,3)));
}

TEST_F(RingTest, isInside){
    EXPECT_TRUE(ring->isInside(Vec2D(0,0)));
    EXPECT_TRUE(ring->isInside(Vec2D(0.5,2)));
    EXPECT_FALSE(ring->isInside(Vec2D(-3,-1)));
    EXPECT_FALSE(ring->isInside(Vec2D(3,4)));
    EXPECT_FALSE(ring->isInside(Vec2D(0,4)));
    EXPECT_FALSE(ring->isInside(Vec2D(3,1)));
}

TEST_F(RingTest, isOnBoundary){
    EXPECT_TRUE(ring->isOnBoundary(Vec2D(0,4)));
    EXPECT_TRUE(ring->isOnBoundary(Vec2D(3,1)));
    EXPECT_FALSE(ring->isOnBoundary(Vec2D(0,0)));
    EXPECT_FALSE(ring->isOnBoundary(Vec2D(-2,3)));
}

TEST_F(RingTest, intersects){
    EXPECT_TRUE(ring->intersects(xAxis));
    EXPECT_TRUE(ring->intersects(yAxis));
    EXPECT_FALSE(ring->intersects(Segment(Vec2D(-2,0),Vec2D(-3,1))));
    EXPECT_FALSE(ring->intersects(Segment(Vec2D(0,0),Vec2D(1,1)))); //fully inside
    EXPECT_TRUE(ring->intersects(Ray(Vec2D(0.5,2),Vec2D(1,1))));
    EXPECT_FALSE(ring->intersects(Segment({2,0},{5,3}))); // tangents edge of ring
    EXPECT_TRUE(ring->intersects(Line<double>::horizontalLine(2)));
    EXPECT_FALSE(ring->intersects(Segment({-1,1},{-4,-1})));
}

TEST_F(RingTest, containsSegment){
    EXPECT_TRUE(ring->contains(Segment({0,4},{0,0})));
    EXPECT_TRUE(ring->contains(Segment({-3,2},{0,1})));
    EXPECT_TRUE(ring->contains(Segment({0,1},{2,0})));
    EXPECT_FALSE(ring->contains(Segment({-1,0},{-1,3})));
    EXPECT_FALSE(ring->contains(Segment({10,10},{11,11})));
    EXPECT_FALSE(ring->contains(Segment({2,3},{4,2})));
    EXPECT_TRUE(ring->contains(Segment({0,0},{0,0})));
    EXPECT_FALSE(ring->contains(Segment({-100,-100},{-100,-100})));
}
#include <fishnet/Line.hpp>
TEST_F(RingTest, intersections){
    std::vector<Vec2D<double>> expected {Vec2D(0,4),Vec2D(0,1),Vec2D(0,-0.5)};
    auto actual = ring -> intersections(yAxis);
    EXPECT_CONTAINS_ALL(actual,expected);
    EXPECT_SIZE(actual, 3);
    std::vector<Vec2D<double>> exp {Vec2D(2,2),Vec2D(4,2)};
    auto act = ring -> intersections(Ray<int>::right(Vec2D(0,2)));
    EXPECT_CONTAINS_ALL(exp,act);
    EXPECT_SIZE(act, 2);
    EXPECT_EMPTY(ring->intersections(Segment(Vec2D(0,5),Vec2D(1,5))));
}

TEST_F(RingTest, area){
    EXPECT_EQ(square->area(),1.0);
    EXPECT_EQ(ring->area(),15.5);
    EXPECT_EQ(convex->area(),2.5);
}

TEST_F(RingTest, centroid){
    EXPECT_EQ(square->centroid(),Vec2D(0.5,0.5));
    EXPECT_EQ(convex->centroid(),Vec2D(4.2,3.8));
    EXPECT_EQ(ring->centroid(),Vec2D(5.0/8.0,13.0/8.0));
}

TEST_F(RingTest, equality){
    EXPECT_NE(*square,*ring);
    auto squareReordered = Ring<int>(std::vector<Vec2D<int>>{Vec2D(1,1),Vec2D(1,0),Vec2D(0,0),Vec2D(0,1),});
    EXPECT_EQ(*square, squareReordered);
    auto squareCCW = Ring<int>(std::vector<Vec2D<int>>{Vec2D(1,1),Vec2D(0,1),Vec2D(0,0),Vec2D(1,0)});
    EXPECT_EQ(*square,squareCCW);
    auto squareBigger = Ring<int>(std::vector<Vec2D<int>>{Vec2D(2,2),Vec2D(0,2),Vec2D(0,0),Vec2D(2,0)});
    EXPECT_NE(*square, squareBigger);
    auto someMatching = Ring<int>(std::vector<Vec2D<int>>{Vec2D(0,0),Vec2D(0,1),Vec2D(1,1),Vec2D(3,1)});
    EXPECT_NE(*square, someMatching);
}

TEST_F(RingTest, crosses){
    EXPECT_FALSE(ring->crosses(*convex));
    EXPECT_FALSE(ring->crosses(*square));
    EXPECT_FALSE(ring->crosses(*ring));
    EXPECT_TRUE(ring->crosses(Ring<int>(std::vector<Vec2D<int>>{{2,1},{4,1},{4,-1},{2,-1}})));
}

TEST_F(RingTest, containsRing){
    EXPECT_TRUE(ring->contains(*square));
    auto triangleInside = Ring<int>(std::vector<Vec2D<int>>{Vec2D{0,2},Vec2D{1,2},Vec2D{1,1}});
    EXPECT_TRUE(ring->contains(triangleInside));
}

    #include <fishnet/ShapeGeometry.hpp>
    static_assert(ShapeGeometry<Ring<double>>);

TEST_F(RingTest, touchesRing){
    EXPECT_FALSE(ring->touches(Ring<int>(std::vector<Vec2D<int>>{{2,1},{4,1},{4,-1},{2,-1}})));
    EXPECT_FALSE(ring->touches(*ring));
    EXPECT_FALSE(ring->touches(*square));
    EXPECT_FALSE(square->touches(*ring));
    EXPECT_TRUE(ring->touches(Ring<int>(std::vector<Vec2D<int>>{{3,3},{4,4},{5,3},{5,2},{4,2}})));
}
#include "ShapeSamples.h"
TEST_F(RingTest, distanceToRing){
    EXPECT_EQ(ring->distance(*convex),1.0);
    EXPECT_EQ(convex->distance(*ring),1.0);
    EXPECT_EQ(ring->distance(*square), -1);
    EXPECT_EQ(ring->distance(Ring<int>(std::vector<Vec2D<int>>{{3,3},{4,4},{5,3},{5,2},{4,2}})),0.0);
    EXPECT_EQ(LinearRingSamples::aaRhombus({0,0},1).distance(LinearRingSamples::aaBB({2,1},{3,-1})),1.0);
}

TEST_F(RingTest, toString){
    EXPECT_EQ(square->toString(), "[(0,0),(0,1)],[(0,1),(1,1)],[(1,1),(1,0)],[(1,0),(0,0)]");
}