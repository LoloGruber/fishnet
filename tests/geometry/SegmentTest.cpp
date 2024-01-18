#include <gtest/gtest.h>
#include "Segment.hpp"
#include "Testutil.h"

using namespace fishnet::geometry;
using namespace fishnet::math;
using namespace testutil;

TEST(SegmentTest, init){
    Segment s {Vec2D(0,0),Vec2D(1,0)};
    EXPECT_EQ(s.p(), Vec2D(0,0));
    EXPECT_EQ(s.q(), Vec2D(1,0));
    EXPECT_TYPE<Vec2D<int>>(s.p());
    EXPECT_TYPE<Vec2D<int>>(s.q());
    Segment r {Vec2D(0.4,0.3),Vec2D(1L,2L)};
    EXPECT_EQ(r.p(), Vec2D(0.4,0.3));
    EXPECT_EQ(r.q(), Vec2D(1L,2L));
    EXPECT_TYPE<Vec2D<double>>(r.p());
    EXPECT_TYPE<Vec2D<double>>(r.q());
    Segment t {Vec2D(1,1),Vec2D(PI,PI)};
    EXPECT_EQ(t.p(), Vec2D(1,1));
    EXPECT_EQ(t.q(),Vec2D(PI,PI));
    EXPECT_TYPE<Vec2D<double>>(t.p());
    EXPECT_TYPE<Vec2D<double>>(t.q());
    Vec2D v {0,1};
    EXPECT_NO_FATAL_FAILURE(Segment(v,v));
}

TEST(SegmentTest, flip){
    Segment s {Vec2D(1,1),Vec2D(2,2)};
    EXPECT_EQ(s.flip(), Segment(Vec2D(2,2),Vec2D(1,1)));
    EXPECT_EQ(s.flip().flip(), s);
}

TEST(SegmentTest, direction){
    EXPECT_EQ(Segment(Vec2D(0,0),Vec2D(1,1)).direction(), Vec2D(1,1));
    EXPECT_EQ(Segment(Vec2D(),Vec2D()).direction(), Vec2D());
    EXPECT_EQ(Segment(Vec2D(2,1),Vec2D(-3,2)).direction(), Vec2D(-5,1));
}

TEST (SegmentTest, upperLowerEndpoint){
    Vec2D p = Vec2D(1,1);
    Vec2D q = Vec2D(1,2);
    Segment s {p,q};
    EXPECT_EQ(s.upperEndpoint(), q);
    EXPECT_EQ(s.lowerEndpoint(),p);
    q= Vec2D(2,1);
    Segment t = Segment(p,q);
    EXPECT_EQ(t.upperEndpoint(),p);
    EXPECT_EQ(t.lowerEndpoint(),q);
}

TEST(SegmentTest, length){
    EXPECT_DOUBLE_EQ(Segment(Vec2D(0,0),Vec2D(1,1)).length(),sqrt(2));
    EXPECT_EQ(Segment(Vec2D(1,0),Vec2D(-4,0)).length(),5);
    EXPECT_EQ(Segment(Vec2D(1,1),Vec2D(1,1)).length(),0);
}

TEST(SegmentTest, isValid){
    EXPECT_TRUE(Segment(Vec2D(1,1),Vec2D(0,0)).isValid());
    EXPECT_TRUE(Segment(Vec2D(0.123,0.456),Vec2D(100000L,2345578L)).isValid());
    EXPECT_FALSE(Segment(Vec2D(),Vec2D()).isValid());
    EXPECT_FALSE(Segment(Vec2D(1.01,1.0),Vec2D(1.01,1.0)).isValid());
    EXPECT_FALSE(Segment(Vec2D(0,0),Vec2D(1e-12,1e-12)).isValid());
}

TEST(SegmentTest, toLine){
    EXPECT_EQ(Segment(Vec2D(0,0),Vec2D(1,1)).toLine(),Line(1,0));
    EXPECT_EQ(Segment(Vec2D(-3,3),Vec2D(2,2)).toLine(),Line(-0.2,2.4));
}

TEST(SegmentTest, isParallel){
    Segment s {Vec2D(2,1),Vec2D(3,2)};
    EXPECT_TRUE(s.isParallel(s));
    EXPECT_TRUE(s.isParallel(Segment(Vec2D(0,0),Vec2D(1,1))));
    EXPECT_TRUE(s.isParallel(Line(1,0)));
    EXPECT_FALSE(s.isParallel(Segment(Vec2D(2,3),Vec2D(3,3))));
    EXPECT_FALSE(s.isParallel(Line(0,2)));
}

TEST(SegmentTest, intersects){
    Segment s {Vec2D(1,1),Vec2D(-2,2)};
    EXPECT_TRUE(s.intersects(Segment(Vec2D(0,0),Vec2D(0,3))));
    EXPECT_TRUE(s.intersects(Segment(Vec2D(2,2),Vec2D(-1,0))));
    EXPECT_TRUE(s.intersects(yAxis));
    EXPECT_FALSE(s.intersects(xAxis));
    EXPECT_FALSE(s.intersects(Segment(Vec2D(-3,-1),Vec2D(2,1))));
}

TEST(SegmentTest, hasOverlay){
    Segment s {Vec2D(0,0),Vec2D(2,2)};
    EXPECT_TRUE(s.hasOverlay(Segment(Vec2D(-1,-1),Vec2D(0.5,0.5))));
    EXPECT_TRUE(s.hasOverlay(s));
    EXPECT_TRUE(s.hasOverlay(Segment(Vec2D(2,2),Vec2D(3,3))));
    EXPECT_FALSE(s.hasOverlay(Segment(Vec2D(2,2),Vec2D(3,2)))); //not parallel
    EXPECT_FALSE(s.hasOverlay(Segment(Vec2D(3,3),Vec2D(4,4)))); // no common points
    EXPECT_FALSE(s.hasOverlay(Segment(Vec2D(-1,-1),Vec2D(-1,0))));
}

TEST(SegmentTest, containsSegment){
    Segment s {Vec2D(0,0),Vec2D(2,2)};
    Segment t {Vec2D(0,0),Vec2D(1,1)};
    EXPECT_TRUE(s.containsSegment(t));
    EXPECT_FALSE(t.containsSegment(s));
    EXPECT_TRUE(s.containsSegment(Segment(Vec2D(0.5,0.5),Vec2D(1.2,1.2))));
    EXPECT_FALSE(s.containsSegment(Segment(Vec2D(0,0),Vec2D(1,2))));
}

TEST(SegmentTest, touches){
    Segment s {Vec2D(0,0),Vec2D(2,2)};
    EXPECT_TRUE(s.touches(Segment(Vec2D(0,0),Vec2D(-1,0))));
    EXPECT_TRUE(s.touches(Segment(Vec2D(2,2),Vec2D(-1000.1,0))));
    EXPECT_FALSE(s.touches(s));
    EXPECT_FALSE(s.touches(Segment(Vec2D(0,0),Vec2D(3,3))));
    EXPECT_FALSE(s.touches(Segment(Vec2D(-2,-2),Vec2D(1,1))));
    EXPECT_FALSE(s.touches(Segment(Vec2D(-1,-1),Vec2D(2,2))));
    EXPECT_FALSE(s.touches(Segment(Vec2D(-100.23,0),Vec2D(0,1000.3))));
}

TEST(SegmentTest, equality){
    Segment s {Vec2D(1,0),Vec2D(2,0)};
    EXPECT_EQ(s,s);
    EXPECT_EQ(s, Segment(Vec2D(1,0),Vec2D(2,0)));
    EXPECT_EQ(Segment(Vec2D(-PI,PI),Vec2D(0,0)),Segment(Vec2D(-PI,PI),Vec2D()));
    EXPECT_EQ(Segment(Vec2D(0.0,0.0),Vec2D(1L,1L)),Segment<float>(Vec2D<float>(0,0),Vec2D(1.0f,1.0f)));
    EXPECT_EQ(s, Segment(Vec2D(2,0),Vec2D(1,0)));
    EXPECT_NE(s, Segment(Vec2D(1,0),Vec2D(2,0.5)));
    EXPECT_NE(s, Segment(Vec2D(2,0),Vec2D(0,0)));
    auto hasher = std::hash<Segment<int>>{};
    EXPECT_EQ(hasher(s),hasher(s));
    EXPECT_EQ(hasher(s),hasher(Segment(Vec2D(2,0),Vec2D(1,0))));
    EXPECT_NE(hasher(s),hasher(Segment(Vec2D(2,0),Vec2D(0,0))));
}

TEST(SegmentTest, contains){
    Segment s {Vec2D(0,0),Vec2D(3,2)};
    EXPECT_TRUE(s.contains(Vec2D(s.p())));
    EXPECT_TRUE(s.contains(Vec2D(s.q())));
    EXPECT_TRUE(s.contains(Vec2D(1.5,1)));
    EXPECT_TRUE(s.contains(Vec2D(1,2.0/3.0)));
    EXPECT_FALSE(s.contains(Vec2D(1,1)));
    EXPECT_FALSE(s.contains(Vec2D(-1.5,-1)));
    EXPECT_TRUE(Segment(Vec2D(),Vec2D()).contains(Vec2D()));
    EXPECT_FALSE(Segment(Vec2D(1,1),Vec2D(1,1)).contains(Vec2D(0.5,0.5)));
}

TEST(SegmentTest, distance){
    Segment s {Vec2D(0,0),Vec2D(2,2)};
    EXPECT_EQ(s.distance(Vec2D(0,0)),0);
    EXPECT_EQ(s.distance(Vec2D(2,2)),0);
    EXPECT_DOUBLE_EQ(s.distance(Vec2D(0,1)),sqrt(2)/2);
    EXPECT_EQ(Segment(Vec2D(0,0),Vec2D(10,0)).distance(Vec2D(5,0.5)),0.5);
    EXPECT_TRUE(areEqual(Segment(Vec2D(2,2),Vec2D(4,2)).distance(Vec2D(5,4)), 2.2360679774998));
}

TEST(SegmentTest, intersection){
    Segment s {Vec2D(0,0),Vec2D(3,2)};
    Segment t {Vec2D(1,2),Vec2D(2,0)};
    auto inter = s.intersection(t);
    EXPECT_EQ(*inter, Vec2D(1.5,1));
    EXPECT_EQ(s.intersection(xAxis).value(),Vec2D(0,0));
    EXPECT_EQ(s.intersection(Line(-4,14)).value(),Vec2D(3,2));
    EXPECT_EQ(s.intersection(Segment(Vec2D(-2,1),Vec2D(0,1))),std::nullopt);
    EXPECT_TRUE(s.intersection(Line(Vec2D(-2,1),Vec2D(0,1))).has_value());
    EXPECT_EQ(s.intersection(Line(2,2)),std::nullopt);
}

TEST(SegmentTest, toString){
    Segment s{Vec2D(0,0),Vec2D(3,2)};
    EXPECT_EQ(s.toString(),"[(0,0),(3,2)]");
    std::cout << "Test console output for Segment: " << s << std::endl;
}