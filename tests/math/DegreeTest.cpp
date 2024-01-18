#include <gtest/gtest.h>
#include "Degrees.hpp"

using namespace fishnet::math;

TEST(DegreeTest, DegreesFactory){
    EXPECT_EQ(Degrees::asin(0).getAngleValue(),0);
    EXPECT_EQ(Degrees::asin(-1).getAngleValue(),270.0);
    EXPECT_EQ(Degrees::asin(1).getAngleValue(),90.0);
    EXPECT_ANY_THROW(Degrees::asin(-3));
    EXPECT_EQ(Degrees::acos(0).getAngleValue(),90.0);
    EXPECT_EQ(Degrees::acos(1).getAngleValue(),0.0);
    EXPECT_EQ(Degrees::acos(-1).getAngleValue(),180.0);
    EXPECT_ANY_THROW(Degrees::acos(1.1));
    EXPECT_EQ(Degrees::atan(0).getAngleValue(),0.0);
    EXPECT_EQ(Degrees::atan(1).getAngleValue(),45.0);
    EXPECT_DOUBLE_EQ(Degrees::atan(sqrt(3)).getAngleValue(), 60.0); // floating point rounding error
    
    EXPECT_EQ(Degrees::atan2(0,1).getAngleValue(),0);
    EXPECT_EQ(Degrees::atan2(1,0).getAngleValue(),90.0);
    EXPECT_EQ(Degrees::atan2(-1,0).getAngleValue(),270.0);
    EXPECT_EQ(Degrees::atan2(1,1).getAngleValue(),45.0);
    EXPECT_EQ(Degrees::atan2(-1,-1).getAngleValue(), 225.0); 
    EXPECT_EQ(Degrees::atan2(0,0).getAngleValue(),0);
}

TEST(DegreeTest, constructor){
    Degrees angle = Degrees(180.0);
    EXPECT_EQ(angle.getAngleValue(),180.0);
    Degrees fromRad = Degrees(Radians(PI/2));
    EXPECT_EQ(fromRad.getAngleValue(),90.0);
    EXPECT_ANY_THROW(Degrees(NAN));
    EXPECT_EQ(Degrees(360.0).getAngleValue(),0.0);
}

TEST(DegreeTest, toRadians){
    Degrees angle {180.0};
    EXPECT_EQ(angle.toRadians().getAngleValue(),PI);
    EXPECT_EQ(Degrees(360.0).toRadians().getAngleValue(),0.0);
    EXPECT_EQ(Degrees(Radians(PI/4)).toRadians().getAngleValue(),PI/4);
    EXPECT_EQ(Degrees(270.0).toRadians(),Radians(3*PI/2));
}

TEST(DegreeTest, sine){
    EXPECT_TRUE(Degrees(180.0).sin() < DOUBLE_EPSILON);
    EXPECT_DOUBLE_EQ(Degrees(90.0).sin(),1.0);
    EXPECT_DOUBLE_EQ(Degrees(270.0).sin(),-1.0);
    EXPECT_DOUBLE_EQ(Degrees(0.0).sin(),0.0);
    EXPECT_DOUBLE_EQ(Degrees(360.0).sin(),0.0);
    EXPECT_DOUBLE_EQ(Degrees(45.0).sin(),sqrt(2)/2);
}

TEST(DegreeTest, cosine){
    EXPECT_TRUE(Degrees(90.0).cos() < DOUBLE_EPSILON);
    EXPECT_DOUBLE_EQ(Degrees(0.0).cos(),1.0);
    EXPECT_DOUBLE_EQ(Degrees(180.0).cos(),-1.0);
    EXPECT_TRUE(Degrees(270.0).cos()<DOUBLE_EPSILON);
    EXPECT_DOUBLE_EQ(Degrees(360.0).cos(),1.0);
    EXPECT_DOUBLE_EQ(Degrees(720.0).cos(),1.0);
    EXPECT_DOUBLE_EQ(Degrees(45.0).cos(),sqrt(2)/2);
}

TEST(DegreeTest, tangent){
    EXPECT_DOUBLE_EQ(Degrees(0).tan(),0.0);
    EXPECT_DOUBLE_EQ(Degrees(45.0).tan(),1.0);
    EXPECT_DOUBLE_EQ(Degrees(60.0).tan(),sqrt(3));
}

TEST(DegreeTest, negate){
    Degrees angle {90.0};
    EXPECT_EQ(Degrees(270.0), -angle);
    Degrees zero {0.0};
    EXPECT_EQ(Degrees(360.0),-zero);
}

TEST(DegreeTest, plusMinus){
    EXPECT_EQ(Degrees(0)+Degrees(180.0),Degrees(180.0));
    EXPECT_EQ(Degrees(315.0)+ Degrees(180.0),Degrees(135.0));
    EXPECT_EQ(Degrees(360.0)+Degrees(360.0),Degrees(360.0));
    EXPECT_EQ(Degrees(360.0)-Degrees(180.0),Degrees(180.0));
    EXPECT_EQ(Degrees(0)-Degrees(90.0),Degrees(270.0));
}

TEST(DegreeTest, plusMinusAssignment){
    Degrees a {180.0};
    a += Degrees(45.0);
    EXPECT_EQ(a, Degrees(225.0));
    Degrees b {270.0};
    b += Degrees(90.0);
    EXPECT_EQ(b, Degrees(360.0));
    Degrees c {360.0};
    c -= Degrees(180.0);
    EXPECT_EQ(c, Degrees(180.0));
}

TEST(DegreeTest, equality){
    EXPECT_EQ(Degrees(0.0),Degrees(360.0));
    EXPECT_EQ(Degrees(0.0),Degrees(1e-12));
    EXPECT_NE(Degrees(180.0),Degrees(181.0));
    EXPECT_NE(Degrees(0.0),Degrees(0.0001));
}

TEST(DegreeTest, lessThan){
    EXPECT_LT(Degrees(0.0),Degrees(1.0));
    EXPECT_LT(Degrees(90.0),Degrees(180.0));
    EXPECT_LT(Degrees(0.0),Degrees(0.0001));
    EXPECT_FALSE(Degrees(0.0) < Degrees(1e-12));
}

TEST(DegreeTest, hash){
    Degrees angle {180.0};
    size_t angleHash = std::hash<double>{}(angle.getAngleValue());
    size_t actual = std::hash<Degrees>{}(angle);
    EXPECT_EQ(actual,angleHash);
}
