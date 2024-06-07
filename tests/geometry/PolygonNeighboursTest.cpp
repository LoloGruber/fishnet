#include <gtest/gtest.h>
#include <fishnet/PolygonNeighbours.hpp>
#include <fishnet/SimplePolygon.hpp>
#include <fishnet/Polygon.hpp>
#include "Testutil.h"
#include "ShapeSamples.h"
#include <unordered_map>

using namespace testutil;
using namespace fishnet::geometry;
using PolygonType = fishnet::geometry::SimplePolygon<double>;

class PolygonNeighboursTest:public ::testing::Test {
protected:
    void SetUp() override {
        polygons = {b1,b2,b3,b4,t1,r1,r2};
    }
    std::vector<PolygonType> polygons;
    PolygonType b1 = SimplePolygonSamples::aaBB({0,0},{2,2});
    PolygonType b2 = SimplePolygonSamples::aaBB({3.5,2},{4,1});
    PolygonType b3 = SimplePolygonSamples::aaBB({2.5,3},{3.5,4});
    PolygonType b4 = SimplePolygonSamples::aaBB({2,-0.5},{2.5,-1});
    PolygonType t1 = SimplePolygonSamples::triangle({5,2},{4,3},{5,3});
    PolygonType r1 = SimplePolygonSamples::aaRhombus({-1,3},1);
    PolygonType r2 = SimplePolygonSamples::aaRhombus({-0.5,-0.5},0.5);
};

struct DistancePredicate {
    double maxDistance;
    bool operator()(Shape auto const & lhs, Shape auto const & rhs)const noexcept{
        return lhs.distance(rhs) <= maxDistance;
    }
};

struct BoxWrapper {
    double maxDistance; // in m
    auto operator()(Shape auto const & shape) const noexcept {
        auto rectangle = Rectangle(shape);
        auto l = rectangle.left() -maxDistance - 0.1;
        auto r = rectangle.right() +maxDistance + 0.1;
        auto t = rectangle.top() + maxDistance + 0.1;
        auto b = rectangle.bottom() - maxDistance- 0.1;
        return BoundingBoxPolygon(shape,Rectangle(l,t,r,b));
    }
};

static std::unordered_map<PolygonType,std::vector<PolygonType>> toMap(std::vector<std::pair<PolygonType,PolygonType>>  & adjacencies) {
    std::unordered_map<PolygonType,std::vector<PolygonType>> map;
    for(auto && [from,to]:adjacencies){
        map.try_emplace(from,std::vector<PolygonType>());
        map.try_emplace(to,std::vector<PolygonType>());
        map.at(from).push_back(to);
        map.at(to).push_back(from);
    }
    return map;
}


TEST_F(PolygonNeighboursTest, simple){
    double maxDistance = 1;
    auto result = findNeighbouringPolygons(polygons,DistancePredicate{maxDistance},BoxWrapper(maxDistance),2);
    auto resultMap = toMap(result);
    EXPECT_CONTAINS_ALL(resultMap.at(b1),b4,r2);
    EXPECT_CONTAINS_ALL(resultMap.at(r1),b1);
    EXPECT_SIZE(resultMap.at(r1),1);
    EXPECT_CONTAINS_ALL(resultMap.at(r2),b1);
    EXPECT_SIZE(resultMap.at(r2),1);
    EXPECT_CONTAINS_ALL(resultMap.at(b2),t1,b3);
    EXPECT_SIZE(resultMap.at(b2),2);
    EXPECT_CONTAINS_ALL(resultMap.at(t1),b2,b3);
    EXPECT_SIZE(resultMap.at(t1),2);
    EXPECT_SIZE(std::views::keys(resultMap),7);
}

// #define TEST_PERFORMANCE false
// #if TEST_PERFORMANCE
// #include <fishnet/VectorLayer.hpp>
// #include <fishnet/PathHelper.h>
// #include <fishnet/StopWatch.h>
// #include <fishnet/WGS84Ellipsoid.hpp>
// #include <fishnet/PolygonDistance.hpp>
// static auto PATH_TO_FILE = fishnet::util::PathHelper::projectDirectory() / std::filesystem::path("data/testing/Punjab_Small/Punjab_Small.shp");

// struct Distance{
//     double distance;
//     bool operator()(Shape auto const & lhs, Shape auto const & rhs){
//         auto [l,r] = closestPoints(lhs,rhs);
//         return fishnet::WGS84Ellipsoid::distance(l,r) <= distance;
//     }
// };


// TEST_F(PolygonNeighboursTest, benchmark){
//     std::vector<Polygon<double>> largeDataset;
//     auto layer = fishnet::VectorLayer<Polygon<double>>::read({PATH_TO_FILE});
//     for(auto && g: layer.getGeometries()) {
//         largeDataset.push_back(std::move(g));
//     }
//     std::cout << "Dataset size:" << fishnet::util::size(largeDataset) << std::endl;

//     double maxDistance = 3000;
//     u_int16_t k = 3;
//     auto distancePredicate = Distance{maxDistance};
//     auto boundingBoxPolygonWrapper = [maxDistance](const Shape auto & settPolygon ){
//             /* Create scaled aaBB containing at least all points reachable from the polygon within the maximum edge distance*/
//             auto aaBB = fishnet::geometry::Rectangle<fishnet::math::DEFAULT_NUMERIC>(settPolygon.aaBB().getPoints());
//             double distanceMetersTopLeftBotLeft = fishnet::WGS84Ellipsoid::distance(aaBB.left(),aaBB.top(),aaBB.left(),aaBB.bottom());
//             double scale = maxDistance / distanceMetersTopLeftBotLeft; 
//             return fishnet::geometry::BoundingBoxPolygon(settPolygon,aaBB.scale(scale));
//     };
//     fishnet::util::StopWatch sweepline {"Sweep line neighbours"};
//     auto res = findNeighbouringPolygons(largeDataset,Distance{maxDistance},boundingBoxPolygonWrapper,k);
//     std::cout << "Sweepline output size: " << fishnet::util::size(res) << std::endl;
//     // filtering for k nearest neighbours still has to happen here to be fair...
//     sweepline.stopAndPrint();
// //     auto boxWrapper = [maxDistance](const Shape auto & shp) {
// //         auto aaBB = fishnet::geometry::Rectangle(shp);
// //         double distanceMetersTopLeftBotLeft = fishnet::WGS84Ellipsoid::distance(aaBB.left(),aaBB.top(),aaBB.left(),aaBB.bottom());
// //         double scale = maxDistance / distanceMetersTopLeftBotLeft; 
// //         return aaBB.scale(scale);
// //     };
// //     fishnet::util::StopWatch quadtree {"Quadtree k nearest neighbours"};
// //     auto other = kNearestNeighbouringPolygons(largeDataset,distancePredicate,boxWrapper,k);
// //     std::cout << "Quadtree output size: " << fishnet::util::size(other) << std::endl;
// //     quadtree.stopAndPrint();
// }
// #endif