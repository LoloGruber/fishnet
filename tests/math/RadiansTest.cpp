#include <gtest/gtest.h>
#include <fishnet/Radians.hpp>

using namespace fishnet::math;

TEST(RadiansTest, RadiansFactory){
    auto fromSin = Radians::asin(0.0);
    EXPECT_EQ(fromSin.getAngleValue(),0.0);
    fromSin = Radians::asin(-1);
    EXPECT_EQ(fromSin.getAngleValue(),3*PI/2);
    fromSin = Radians::asin(1);
    EXPECT_EQ(fromSin.getAngleValue(),PI/2);
    EXPECT_ANY_THROW(Radians::asin(-2));

    auto fromCos = Radians::acos(0.0);
    EXPECT_EQ(fromCos.getAngleValue(),PI/2);
    EXPECT_EQ(Radians::acos(-1).getAngleValue(),PI);
    EXPECT_EQ(Radians::acos(1).getAngleValue(),0.0);
    EXPECT_ANY_THROW(Radians::acos(3.14));

    EXPECT_EQ(Radians::atan(0).getAngleValue(),0);
    EXPECT_EQ(Radians::atan(1).getAngleValue(),PI/4);
    EXPECT_EQ(Radians::atan(sqrt(3)).getAngleValue(),PI/3);

    EXPECT_EQ(Radians::atan2(0,1).getAngleValue(),0);
    EXPECT_EQ(Radians::atan2(1,0).getAngleValue(),PI/2);
    EXPECT_EQ(Radians::atan2(-1,0).getAngleValue(),3*PI/2);
    EXPECT_EQ(Radians::atan2(1,1).getAngleValue(),PI/4);
    EXPECT_EQ(Radians::atan2(-1,-1).getAngleValue(), 5*PI/4); // atan(-1/-1) - PI = atan(1)-PI = PI/4 - PI = - 3/4 PI == 5/4 PI
    EXPECT_EQ(Radians::atan2(0,0).getAngleValue(),0);

    EXPECT_EQ(Radians::PI.getAngleValue(),PI);
}

TEST(RadiansTest, constructor){
    Radians angle = Radians(PI);
    EXPECT_EQ(angle.getAngleValue(),PI);
    Radians fromDeg = Radians(Degrees(90.0));
    EXPECT_EQ(fromDeg.getAngleValue(),PI/2);
    EXPECT_ANY_THROW(Radians(NAN));
}

TEST(RadiansTest, toDegrees){
    Radians angle {TWO_PI};
    EXPECT_EQ(angle.toDegrees().getAngleValue(),0.0);
    EXPECT_EQ(Radians(PI).toDegrees().getAngleValue(),180.0);
    EXPECT_EQ(Radians(Degrees(45.0)).toDegrees().getAngleValue(),45.0);
    EXPECT_EQ(Radians((3*PI)/2).toDegrees(),Degrees(270.0)) << Radians((3*PI)/2).toDegrees().getAngleValue();
}

TEST(RadiansTest, sine){
    EXPECT_DOUBLE_EQ(Radians(0).sin(),0.0);
    EXPECT_DOUBLE_EQ(Radians(PI/2).sin(),1.0);
    EXPECT_TRUE(Radians(PI).sin() < DOUBLE_EPSILON); //floating point rounding
    EXPECT_DOUBLE_EQ(Radians(3*PI/2).sin(),-1.0);
    EXPECT_DOUBLE_EQ(Radians(TWO_PI).sin(),0.0);
    EXPECT_DOUBLE_EQ(Radians(PI/4).sin(),sqrt(2)/2);
}

TEST(RadiansTest, cosine){
    EXPECT_DOUBLE_EQ(Radians(0).cos(), 1.0);
    EXPECT_TRUE(Radians(PI/2).cos()< DOUBLE_EPSILON);
    EXPECT_TRUE(Radians(3*PI/2).cos()< DOUBLE_EPSILON);
    EXPECT_DOUBLE_EQ(Radians(TWO_PI).cos(), 1.0);
    EXPECT_DOUBLE_EQ(Radians(PI).cos(), -1.0);
    EXPECT_DOUBLE_EQ(Radians(PI/4).cos(), sqrt(2)/2);
}

TEST(RadiansTest, tangent){
    EXPECT_DOUBLE_EQ(Radians(0).tan(),0.0);
    EXPECT_DOUBLE_EQ(Radians(PI/4).tan(),1.0);
    EXPECT_DOUBLE_EQ(Radians(PI/3).tan(),sqrt(3));
}

TEST(RadiansTest, negate){
    Radians angle {PI/2};
    EXPECT_EQ(Radians(3*PI/2), -angle);
    Radians zero {0.0};
    EXPECT_EQ(Radians(TWO_PI),-zero);
}

TEST(RadiansTest, plusMinus){
    EXPECT_EQ(Radians(0)+Radians(PI),Radians(PI));
    EXPECT_EQ(Radians(7*PI/4)+ Radians(PI),Radians(3*PI/4));
    EXPECT_EQ(Radians(TWO_PI)+Radians(TWO_PI),Radians(TWO_PI));
    EXPECT_EQ(Radians(TWO_PI)-Radians(PI),Radians(PI));
    EXPECT_EQ(Radians(0)-Radians(PI/2),Radians(3*PI/2));
}

TEST(RadiansTest, plusMinusAssignment){
    Radians a {PI};
    a += Radians(PI/4);
    EXPECT_EQ(a, Radians(5*PI/4));
    Radians b {3*PI/2};
    b +=  Radians(PI/2);
    EXPECT_EQ(b, Radians(TWO_PI));
    Radians c {TWO_PI};
    c -= Radians{PI};
    EXPECT_EQ(c, Radians(PI));
}

TEST(RadiansTest, equality ){
    EXPECT_EQ(Radians(0.0),Radians(TWO_PI));
    EXPECT_EQ(Radians(0.0),Radians(1e-12));
    EXPECT_NE(Radians(PI),Radians(PI/2));
    EXPECT_NE(Radians(0.0),Radians(0.0001));
}

TEST(RadiansTest, lessThan){
    EXPECT_LT(Radians(0.0),Radians(1.0));
    EXPECT_LT(Radians(PI/2),Radians(PI));
    EXPECT_LT(Radians(0.0),Radians(0.0001));
    EXPECT_FALSE(Radians(0.0) < Radians(1e-12));
}

TEST(RadiansTest, hash){
    Radians angle {PI};
    size_t angleHash = std::hash<double>{}(angle.getAngleValue());
    size_t radiansHash = std::hash<Radians>{}(angle);
    EXPECT_EQ(radiansHash,angleHash);
}

