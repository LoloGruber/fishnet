#include "CharacteristicShape.h"
#include <gtest/gtest.h>
#include "Polygon.h"
#include <random>
#include "StopWatch.h"
#include <array>
//todo put in library
static void sortCounterClockwise(std::vector<Vec2D> & points){
    if(points.size() <= 1) return;
    Vec2D center;
    for(const auto & p : points) {
        center = center + p;
    }
    center = center / points.size();
    std::sort(points.begin(),points.end(), [&center](const Vec2D & p, const Vec2D & q ){
        if (p == center){
            return false;
        }
        if(q == center){
            return false;
        }
        return p.angle(center) < q.angle(center);
    });
}
using PointVector = std::vector<Vec2D>;

static std::vector<std::pair<PointVector,PointVector>> polygonTestSet(u_int32_t amount){
    u_int32_t minVertices =4;
    u_int32_t maxVertices = 25;
    std::uniform_int_distribution<int> xRand(1,11);
    std::uniform_int_distribution<int> yRand(1,11);
    std::uniform_int_distribution<int> nodesInPolygon(minVertices,maxVertices);
    std::default_random_engine re;
    std::vector<std::pair<PointVector,PointVector>> testSet;

    for(int i = 0 ; i < amount; i++){
        int nodesInFirst = nodesInPolygon(re);
        int nodesInSecond = nodesInPolygon(re);
        std::vector<Vec2D> first;
        first.reserve(nodesInFirst) ;
        std::vector<Vec2D> second;
        second.reserve(nodesInSecond);
        for(int j = 0; j < nodesInFirst;j++){
            double x = xRand(re) -5;
            double y = yRand(re) -5;
            Vec2D res {x,y};
            if(std::find(first.begin(),first.end(),res) == first.end() ){
                first.push_back(res);
            }else {
                j--;
            }
        }
        sortCounterClockwise(first);
        first.push_back(first.front());
        for(int j = 0; j < nodesInSecond;j++){
            double x = xRand(re) -5;
            double y = yRand(re) -5;
            Vec2D res{x,y};
            if(std::find(first.begin(),first.end(),res) == first.end()){
                first.push_back(res);
            }else {
                j--;
            }
        }
        sortCounterClockwise(second);
        second.push_back(second.front());
        testSet.push_back(std::make_pair<PointVector,PointVector>(std::move(first),std::move(second)));
    }
    return testSet;
}



TEST(CharacteristicShapeTest, PerformanceTest){
    u_int32_t samples = 10000;
    auto polygons = polygonTestSet(samples);
    std::vector<double> timeResults;
    for(const auto & pair: polygons) {
        StopWatch watch;
        std::vector<Vec2D> combinedPoints;
        for(auto &p: pair.first){
            combinedPoints.push_back(p);
        }
        for(auto &p: pair.second){
            combinedPoints.push_back(p);
        }
        auto result = CharacteristicShape::charactersticConcaveHull(combinedPoints) ;
        auto p = Polygon::create(result);
        timeResults.push_back(watch.stop());
    }
    double sum = std::reduce(timeResults.begin(),timeResults.end());
    std::cout << "Total: " << sum << std::endl;
    std::cout << "Average: " << (sum/samples) << std::endl;
}