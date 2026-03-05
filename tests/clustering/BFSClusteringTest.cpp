#include <fishnet/TestUtil.hpp>
#include <fishnet/Vec2D.hpp>
#include <fishnet/BFSClustering.hpp>
#include "ClusteringTestUtil.hpp"

class BFSClusteringTest: public ::testing::Test{
protected:
    static inline std::vector<fishnet::geometry::Vec2DStd> points = {
        {0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0},
        {3.0, 3.0}, {4.0, 3.0}, {3.0, 4.0}, {4.0, 4.0},
        {10.0, 10.0}, {11.0, 10.0}, {10.0, 11.0},
        {8.0, 8.0},{8.0,8.5},
        {-5.0,-5.0}
    };
};

TEST_F(BFSClusteringTest, NoRelationPredicate){
    auto graph = completeGraph(points);
    fishnet::BFSClustering<fishnet::geometry::Vec2DStd> bfsClustering;
    auto result = bfsClustering(graph);
    EXPECT_EQ(result.clusters.size(), 1);
    testutil::EXPECT_EMPTY(result.noise);
}

TEST_F(BFSClusteringTest, DistanceRelationPredicate){
    auto distancePredicate = [](const fishnet::geometry::Vec2DStd & a, const fishnet::geometry::Vec2DStd & b){
        return a.distance(b) <= 1.5;
    };
    auto graph = completeGraph(points);
    fishnet::BFSClustering<fishnet::geometry::Vec2DStd> bfsClustering (std::move(distancePredicate));
    auto result = bfsClustering(graph);
    EXPECT_EQ(result.clusters.size(), 5);
    testutil::EXPECT_EMPTY(result.noise);
}