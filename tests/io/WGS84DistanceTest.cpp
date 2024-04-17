#include <gtest/gtest.h>
#include <fishnet/WGS84Ellipsoid.hpp>
using namespace fishnet;

TEST(WGS84EllipsoidTest, flattening) {
    double flatteningExpected = 0.00335281066474748;
    EXPECT_DOUBLE_EQ(WGS84Ellipsoid::flattening,flatteningExpected);
}

TEST(WGS84EllipsoidTest, radius){
    double expectedRadiusKM = 6378.137;
    EXPECT_DOUBLE_EQ(WGS84Ellipsoid::radiusInMeter,expectedRadiusKM*1000);
}


TEST(WGS84EllipsoidTest, TokioBerlin){
    double phiBerlin = 52.516666666666667;
    double lambdaBerlin = 13.400;
    double phiTokio = 35.700;
    double lambdaTokio = 139.76666666666667;
    double distanceTokioBerlinInMeters = 8941202.50458698;
    EXPECT_DOUBLE_EQ(WGS84Ellipsoid::distance(lambdaBerlin,phiBerlin,lambdaTokio,phiTokio),distanceTokioBerlinInMeters);
}

TEST(WGS84EllipsoidTest, TokioBerlinApprox){
    double phiBerlin = 52.516666666666667;
    double lambdaBerlin = 13.400;
    double phiTokio = 35.700;
    double lambdaTokio = 139.76666666666667;
    double distanceTokioBerlinInMeters = 8928954.1420394;
    EXPECT_DOUBLE_EQ(WGS84Ellipsoid::distance(lambdaBerlin,phiBerlin,lambdaTokio,phiTokio, false),distanceTokioBerlinInMeters);
}