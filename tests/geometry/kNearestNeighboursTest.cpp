#include <gtest/gtest.h>
#include "NearestNeighbours.hpp"
#include "Testutil.h"

using namespace fishnet::geometry;
using namespace testutil;

class kNearestNeighboursTest: public ::testing::Test{
protected:
    void SetUp() override {
        points = std::vector<Vec2D<double>> {
            A, B,C,D,E,F,G,H,I,J,K,L,M
        };
    }
    Vec2D<double> A {0,3};
    Vec2D<double> B {1,3.5};
    Vec2D<double> C {-1,2};
    Vec2D<double> D {-1.5,4.5};
    Vec2D<double> E {0,6};
    Vec2D<double> F {-2,1};
    Vec2D<double> G {2,1};
    Vec2D<double> H {3,3};
    Vec2D<double> I {4,3};
    Vec2D<double> J {3,0};
    Vec2D<double> K {6,1};
    Vec2D<double> L {-4,3};
    Vec2D<double> M {-12,-4};
    std::vector<Vec2D<double>> points;
};

TEST_F(kNearestNeighboursTest, OneNeighbour){
    auto res = AllKNearestNeighbours<double>(points,1);
    for(const auto & [point,neighbours] : res){
        if(point == A) {
            EXPECT_EQ(neighbours[0],B);
        }
    }
}

TEST_F(kNearestNeighboursTest, TwoNeighbours){
    auto res = AllKNearestNeighbours<double>(points,2);
    for(const auto & [point,neighbours] : res){
        if(point == A) {
            EXPECT_CONTAINS_ALL(neighbours,std::vector<Vec2D<double>>{{B,C}});
        }
    }
}

TEST_F(kNearestNeighboursTest, SingleNeighbourAvailable){
    std::set<Vec2D<double>,LexicographicOrder> points;
    points.insert(Vec2D(0,0));
    EXPECT_EQ(nearestNeighbour(Vec2D<double>(1,1),points),Vec2D<double>(0,0));
}
