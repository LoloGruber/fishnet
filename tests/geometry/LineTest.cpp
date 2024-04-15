#include <gtest/gtest.h>
#include <fishnet/Line.hpp>
#include <iostream>
using namespace fishnet::geometry;

TEST(LineTest, InitTwoPoints){
    auto l1 = Line(Vec2D{0,0},Vec2D(1,0));
    EXPECT_EQ(l1.p,Vec2D(0,0));
    EXPECT_EQ(l1.q,Vec2D(1,0));
    EXPECT_EQ(Line(Vec2D(1,1),Vec2D(1.11,1.11)).q, Vec2D(1.11,1.11));
    EXPECT_EQ(Line(Vec2D(1.11,1.11),Vec2D(0,0)).q, Vec2D(0,0));
    EXPECT_ANY_THROW(Line(Vec2D(),Vec2D()));
}

TEST(LineTest, InitSlopeYIntercept){
    auto l = Line(1,0);
    EXPECT_EQ(l.p, Vec2D(0,0) );
    EXPECT_EQ(l.q , Vec2D(1,1));
    auto m = Line(3.5,2);
    EXPECT_EQ(m.p,Vec2D(0,2));
    EXPECT_EQ(m.q, Vec2D(1,5.5));
    auto n = Line(-3,2.5);
    EXPECT_EQ(n.p, Vec2D(0,2.5));
    EXPECT_EQ(n.q, Vec2D(1,-0.5));
}

TEST(LineTest, CoordinateAxis){
    EXPECT_TRUE(Line<>::Y_AXIS.direction().isOrthogonal(Line<>::X_AXIS.direction()));
    EXPECT_EQ(Line<>::Y_AXIS.p.x,0);
    EXPECT_EQ(Line<>::Y_AXIS.q.x,0);
    EXPECT_EQ(Line<>::X_AXIS.p.y,0);
    EXPECT_EQ(Line<>::X_AXIS.q.y,0);
}

TEST(LineTest, toLine){
    auto l = Line(1,0);
    EXPECT_EQ(l.toLine(),l);
}

TEST(LineTest, direction){
    auto line = Line(Vec2D(0.5,0.5),Vec2D(1,1));
    EXPECT_EQ(line.direction(),Vec2D(0.5,0.5));
}

TEST(LineTest, isVertical){
    Line l {Vec2D(1,1),Vec2D(1,100)};
    EXPECT_TRUE(l.isVertical());
    EXPECT_TRUE(Line<>::Y_AXIS.isVertical());
    EXPECT_FALSE(Line(1,0).isVertical());
    EXPECT_FALSE(Line<>::X_AXIS.isVertical());
}

TEST(LineTest, slope){
    Line l {Vec2D(0,0),Vec2D(2,1)};
    EXPECT_EQ(l.slope(),0.5);
    EXPECT_EQ(Line<>::Y_AXIS.slope(),std::numeric_limits<double>::max());
    EXPECT_EQ(Line(Vec2D(1,1),Vec2D(1,2)).slope(), std::numeric_limits<double>::max());
    EXPECT_EQ(Line(2,-1).slope(), 2);
    EXPECT_EQ(Line<>::X_AXIS.slope(),0);
    EXPECT_EQ(Line<int>::verticalLine(-1).slope(), std::numeric_limits<double>::max());
    EXPECT_EQ(Line<int>::horizontalLine(1).slope(),0);
}  

TEST(LineTest, yIntercept){
    Line l {Vec2D(2,1), Vec2D(-1,0)};
    EXPECT_TRUE(l.yIntercept().has_value());
    EXPECT_DOUBLE_EQ(l.yIntercept().value(),1.0/3.0);
    EXPECT_EQ(yAxis.yIntercept(),std::nullopt);
    EXPECT_EQ(Line(1,-5).yIntercept(),-5);
    EXPECT_EQ(Line<int>::verticalLine(1).yIntercept(),std::nullopt);
    EXPECT_EQ(Line<int>::horizontalLine(4).yIntercept().value(), 4);
}

TEST(LineTest, contains){
    EXPECT_TRUE(xAxis.contains(Vec2D()));
    EXPECT_TRUE(xAxis.contains(Vec2D(-10,0)));
    EXPECT_TRUE(yAxis.contains(Vec2D()));
    EXPECT_TRUE(yAxis.contains(Vec2D(0,3)));
    EXPECT_FALSE(yAxis.contains(Vec2D(1,0)));
    Line l (0.5,2);
    EXPECT_TRUE(l.contains(Vec2D(1,2.5)));
    EXPECT_FALSE(l.contains(Vec2D(1,2)));
    EXPECT_TRUE(l.contains(l.intersection(xAxis).value()));
}

TEST(LineTest, isLeftisRight) {
    EXPECT_TRUE(yAxis.isLeft(Vec2D<double>{-1,0}));
    EXPECT_TRUE(yAxis.isRight(Vec2D<double>{1,0}));
    EXPECT_FALSE(yAxis.isLeft(Vec2D(0,0)));
    EXPECT_TRUE(xAxis.isLeft(Vec2D(0,1)));
    EXPECT_TRUE(xAxis.isRight(Vec2D(0,-1)));
    EXPECT_TRUE(Line(1,0).isLeft(Vec2D(0,1)));
}

#include <fishnet/Segment.hpp>
TEST(LineTest, isParallel){
    EXPECT_TRUE(xAxis.isParallel(Line<double>::horizontalLine(1)));
    EXPECT_TRUE(Line(1,0).isParallel(Line(1,1)));
    EXPECT_FALSE(Line(1,0).isParallel(Line(2,0)));
    EXPECT_TRUE(yAxis.isParallel(Line<double>::verticalLine(0.5)));
    EXPECT_TRUE(Line(1,0).isParallel(Segment(Vec2D(1,0),Vec2D(2,1))));
    EXPECT_FALSE(Line(1,0).isParallel(Segment(Vec2D(1,1),Vec2D(2,1))));
}

TEST(LineTest, intersects){
    EXPECT_TRUE(xAxis.intersects(yAxis));
    EXPECT_TRUE(Line(1.11,0).intersects(Line(3,-1)));
    EXPECT_FALSE(Line<int>::horizontalLine(2).intersects(Line<int>::horizontalLine(-3)));
    EXPECT_FALSE(yAxis.intersects(Line<int>::verticalLine(3000)));
    EXPECT_TRUE(Line(1,0).intersects(Segment(Vec2D(0,2),Vec2D(2,2))));
    EXPECT_FALSE(Line(1,0).intersects(Segment(Vec2D(2,0),Vec2D(3,0))));
}

TEST(LineTest, intersection){
    auto l = Line(Vec2D(0,0),Vec2D(2,2));
    auto m = Line(Vec2D(2,0),Vec2D(0,2));
    auto inter = l.intersection(m);
    EXPECT_EQ(*inter,Vec2D(1,1));
    EXPECT_EQ(l.intersection(Segment(Vec2D(-2,0),Vec2D(-1,0))),std::nullopt);
    EXPECT_EQ(l.intersection(Line(Vec2D(-1,0),Vec2D(1,2))),std::nullopt);
}

TEST(LineTest, equality){
    EXPECT_EQ(Line(1,0),Line(1,0));
    EXPECT_EQ(Line(Vec2D(-1,-1),Vec2D(0,0)),Line(Vec2D(0,0),Vec2D(0.5,0.5)));
    EXPECT_EQ(std::hash<Line<int>>{}(Line(Vec2D(-1,-1),Vec2D(0,0))),std::hash<Line<double>>{}(Line(Vec2D(0,0),Vec2D(0.5,0.5))));
    EXPECT_NE(Line(1,0),Line(-1,0));
    EXPECT_NE(Line<int>::verticalLine(1),yAxis);
    EXPECT_EQ(Line<int>::horizontalLine(0),xAxis);
    EXPECT_EQ(std::hash<Line<int>>{}(Line<int>::horizontalLine(0)),std::hash<Line<double>>{}(xAxis));
    EXPECT_EQ(Line(Vec2D(0,2),Vec2D(1,0)),Line(-2,2));

}

TEST(LineTest, toString){
    Line l {1,2};
    EXPECT_EQ(l.toString(), "Line y = 1.000000 * x + 2.000000");
    EXPECT_EQ(yAxis.toString(), "Vertical Line x = 0.000000");
    std::cout <<"Testing Console Output: "<< l << std::endl; 
}

