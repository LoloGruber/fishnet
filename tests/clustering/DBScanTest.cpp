#include <fishnet/TestUtil.hpp>
#include <fishnet/DBScan.hpp>
#include <fishnet/Vec2D.hpp>
#include "ClusteringTestUtil.hpp"

class DBScanTest: public ::testing::Test{
protected:
    static inline std::vector<fishnet::geometry::Vec2DStd> points = {
        {0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0},
        {3.0, 3.0}, {4.0, 3.0}, {3.0, 4.0}, {4.0, 4.0},
        {10.0, 10.0}, {11.0, 10.0}, {10.0, 11.0},
        {8.0, 8.0},{8.0,8.5},
        {-5.0,-5.0}
    };
};

TEST_F(DBScanTest, BasicClustering){
    auto graph = completeGraph<fishnet::geometry::Vec2DStd>(points);
    auto distanceFunction = [](const fishnet::geometry::Vec2DStd & a, const fishnet::geometry::Vec2DStd & b){
        return a.distance(b);
    };
    fishnet::DBSCAN<fishnet::geometry::Vec2DStd> dbscan(1.5, 2, std::move(distanceFunction));
    auto result = dbscan.cluster(graph);
    EXPECT_EQ(result.clusters.size(), 3);
    EXPECT_EQ(result.noise.size(), 3);
}
