#include <gtest/gtest.h>
#include <fishnet/OGRGeometryAdapter.hpp>
#include "Testutil.h"

using namespace testutil;

TEST(OGRAdapterTest, Point){
    auto ogrP = fishnet::OGRGeometryAdapter::OGRUniquePtr<OGRPoint>(new OGRPoint(2.5,-5));
    auto p = fishnet::OGRGeometryAdapter::fromOGR(*ogrP);
    EXPECT_EQ(p, Vec2D<double>(2.5,-5));
    auto convertedBack = fishnet::OGRGeometryAdapter::toOGR(p);
    EXPECT_EQ(*ogrP,*convertedBack);
}

TEST(OGRAdapterTest, Ring) {
    auto p = fishnet::OGRGeometryAdapter::OGRUniquePtr<OGRPoint>(new OGRPoint(1,1));
    auto p2 = fishnet::OGRGeometryAdapter::OGRUniquePtr<OGRPoint>(new OGRPoint(2,2));
    auto p3 = fishnet::OGRGeometryAdapter::OGRUniquePtr<OGRPoint>(new OGRPoint(2,1));
    auto ogrRing = fishnet::OGRGeometryAdapter::OGRUniquePtr<OGRLinearRing>(new OGRLinearRing());
    ogrRing->addPoint(p.get());
    ogrRing->addPoint(p2.get());
    ogrRing->addPoint(p3.get());
    auto fsnRing = fishnet::OGRGeometryAdapter::fromOGR(*ogrRing);
    auto points = std::vector<Vec2D<int>> {{1,1},{2,2},{2,1}};
    auto expected = Ring<int>(points);
    EXPECT_EQ(fsnRing, expected);
    
    auto convertedBack = fishnet::OGRGeometryAdapter::toOGR(expected);
    std::vector<OGRPoint> ogrPoints;
    for(const auto & ogrP : convertedBack.get()) {
        ogrPoints.push_back(ogrP);
    }
    EXPECT_EQ(ogrPoints[0],*p);
    EXPECT_EQ(ogrPoints[1],*p2);
    EXPECT_EQ(ogrPoints[2],*p3);
}

TEST(OGRAdapterTest, Polygon ){
    auto polygon = OGRPolygon();
    auto extRing = OGRLinearRing();
    extRing.addPoint(0,0);
    extRing.addPoint(0,2);
    extRing.addPoint(2,2);
    extRing.addPoint(2,0);
    polygon.addRing(&extRing);
    auto intRing = OGRLinearRing();
    intRing.addPoint(0.5,0.5);
    intRing.addPoint(0.5,1.5);
    intRing.addPoint(1.5,1.5);
    intRing.addPoint(1.5,0.5);
    polygon.addRing(&intRing);
    auto fishnetPolygon = fishnet::OGRGeometryAdapter::fromOGR(polygon);
    Ring<int> expectedBoundary {std::vector<Vec2D<int>>{{0,0},{0,2},{2,2},{2,0}}};
    EXPECT_EQ(fishnetPolygon->getBoundary(),expectedBoundary);
    Ring<double> expectedInterior {std::vector<Vec2D<double>>{{0.5,0.5},{0.5,1.5},{1.5,1.5},{1.5,0.5}}};
    for(const auto & interiorRing : fishnetPolygon->getHoles()){
        EXPECT_EQ(interiorRing,expectedInterior);
    }

    auto ogrConvertedBack = fishnet::OGRGeometryAdapter::toOGR(fishnetPolygon.value());
    std::vector<OGRPoint> actualBoundary;
    std::vector<OGRPoint> expectedBoundaryPoints {{OGRPoint(0,0),OGRPoint(0,2),OGRPoint(2,2),OGRPoint(2,0),OGRPoint(0,0)}};
    for(const auto & p : ogrConvertedBack->getExteriorRing()) {
        actualBoundary.push_back(p);
    }
    EXPECT_RANGE_EQ(actualBoundary,expectedBoundaryPoints);
}