#include <gtest/gtest.h>
#include <fishnet/WGS84Ellipsoid.hpp>
#include <fishnet/Rectangle.hpp>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/PathHelper.h>
#include "Testutil.h"
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
    geometry::Vec2DReal berlin = {lambdaBerlin,phiBerlin};
    geometry::Vec2DReal tokio = {lambdaTokio, phiTokio};
    EXPECT_DOUBLE_EQ(WGS84Ellipsoid::distance(berlin,tokio),distanceTokioBerlinInMeters);
}

TEST(WGS84EllipsoidTest, TokioBerlinApprox){
    double phiBerlin = 52.516666666666667;
    double lambdaBerlin = 13.400;
    double phiTokio = 35.700;
    double lambdaTokio = 139.76666666666667;
    double distanceTokioBerlinInMeters = 8928954.1420394;
    EXPECT_DOUBLE_EQ(WGS84Ellipsoid::distance(lambdaBerlin,phiBerlin,lambdaTokio,phiTokio, false),distanceTokioBerlinInMeters);
}

TEST(WGS84EllipsoidTest, SquareKilometerArea){
    auto layer = fishnet::VectorLayer<geometry::Polygon<double>>::read({util::PathHelper::projectDirectory() / std::filesystem::path("data/output/small_dataset/Punjab_Small.shp")});
    geometry::Polygon<double> min = {Ring(std::vector<Vec2DReal>{{100.0,0.0},{100.0,100.0},{0.0,0.0}})};
    for(const auto & p : layer.getGeometries()){
        if(p.area() < min.area())
            min = p;
    }
    std::cout << min.area() << std::endl;
    std::cout << WGS84Ellipsoid::area(min) << std::endl;
    std::cout << min << std::endl;
    testutil::TODO();
}