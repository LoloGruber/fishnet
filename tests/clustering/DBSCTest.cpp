#include <fishnet/TestUtil.hpp>
#include <fishnet/Vec2D.hpp>
#include <fishnet/DBSC.hpp>
#include "ClusteringTestUtil.hpp"

using T = fishnet::geometry::Vec2DStd;
using G = fishnet::graph::UndirectedGraph<T>;
using namespace testutil;

struct PointDistanceFunction {
    static double operator()(const T & lhs, const T & rhs) {
        return lhs.distance(rhs);
    }
};

void EXPECT_CONTAINS_CLUSTER(const std::vector<std::vector<T>> & clusters, const fishnet::util::forward_range_of<T> auto & expectedCluster){
    for(const auto & c : clusters){
        auto errorMessage = unsortedRangeEqual(c, expectedCluster);
        if(not errorMessage.has_value()){
            SUCCEED();
            return;
        }
    }
    std::stringstream ss;
    ss << "[";
    for(const auto & node : expectedCluster){
        ss << node << ", ";
    }
    ss << "]";
    FAIL() << "Expected cluster not found: " << ss.str();
}

class DBSCTest: public ::testing::Test{
protected:
    static inline std::vector<T> c1 = {{0,0}, {0,1}, {1,0}, {1,1},{0.5,0.5}};
    static inline std::vector<T> c2 = {{5,5}, {5,6}, {6,5}, {6,6}};
    static inline std::vector<T> c3 = {{2.5,2.5}, {2.5,3}, {3,2.5}};
    static inline std::vector<T> c4 = {{-5,-5}, {-4,-4}, {-4,-5}, {-5,-4}};
    static inline std::vector<T> c5 = {{10,10},{10,11}};
    static inline std::vector<T> all_nodes = [](){
        std::vector<T> nodes;
        nodes.insert(nodes.end(), c1.begin(), c1.end());
        nodes.insert(nodes.end(), c2.begin(), c2.end());
        nodes.insert(nodes.end(), c3.begin(), c3.end());
        nodes.insert(nodes.end(), c4.begin(), c4.end());
        nodes.insert(nodes.end(), c5.begin(), c5.end());
        return nodes;
    }();
    G graph;


    void SetUp() override {
        graph = completeGraph(all_nodes);
    }
 };

 TEST_F(DBSCTest, DefaultClusteringNoAttribute){
    fishnet::DBSC<T> dbsc(2.0,2,3, PointDistanceFunction{}, [](const auto & node){return 0.0;});
    auto result = dbsc(graph);
    EXPECT_SIZE(result.clusters, 4);
    EXPECT_CONTAINS_CLUSTER(result.clusters, c1);
    EXPECT_CONTAINS_CLUSTER(result.clusters, c2);
    EXPECT_CONTAINS_CLUSTER(result.clusters, c3);
    EXPECT_CONTAINS_CLUSTER(result.clusters, c4);
    // EXPECT_CONTAINS_CLUSTER(result.clusters, c5); not contained since only two nodes, so it is considered noise
    EXPECT_SIZE(result.noise, 2);
 }

 TEST_F(DBSCTest, DefaultClusteringNoAttributeBuilder){
    auto dbsc = fishnet::DBSCBuilder<T>()
        .setEps(2.0)
        .setBeta(2)
        .setMinPts(2) // now cluster c5 should be detected as a cluster since minPts is 2
        .setDistanceFunction(PointDistanceFunction{})
        .setAttributeExtractor([](const auto & node){return 0.0;})
        .build();
    auto result = dbsc(graph);
    EXPECT_SIZE(result.clusters, 5);
    EXPECT_CONTAINS_CLUSTER(result.clusters, c1);
    EXPECT_CONTAINS_CLUSTER(result.clusters, c2);
    EXPECT_CONTAINS_CLUSTER(result.clusters, c3);
    EXPECT_CONTAINS_CLUSTER(result.clusters, c4);
    EXPECT_CONTAINS_CLUSTER(result.clusters, c5);
    EXPECT_EMPTY(result.noise);
 }

 TEST_F(DBSCTest, ClusteringWithAttribute){
    std::unordered_map<T, double> attributes;
    for(const auto & node : c1){
        attributes[node] = 1;
    }
    attributes[c1[0]] = 10; // one node in c1 has a very different attribute value, so it should be considered noise

    auto dbsc = fishnet::DBSCBuilder<T>()
        .setEps(2.0)
        .setBeta(2)
        .setMinPts(2)
        .setDistanceFunction(PointDistanceFunction{})
        .setAttributeExtractor([&attributes](const auto & node){return attributes.at(node);})
        .build();
    auto inputGraph = completeGraph(c1);
    auto result = dbsc(inputGraph);
    EXPECT_SIZE(result.clusters, 1);
    EXPECT_CONTAINS_CLUSTER(result.clusters, c1 | std::views::drop(1)); // only the node with the different attribute value should be considered noise, the other four nodes should still be clustered together
    EXPECT_SIZE(result.noise, 1);
    EXPECT_CONTAINS(result.noise, c1[0]);  
 }

 TEST_F(DBSCTest, ClusteringWithCustomT1){
    std::unordered_map<T, double> attributes;
    std::ranges::for_each(c1, [&attributes](const auto & node){attributes[node] = 0;});
    std::ranges::for_each(c2, [&attributes](const auto & node){attributes[node] = 10;});
    std::ranges::for_each(c3, [&attributes](const auto & node){attributes[node] = 2;});
    std::ranges::for_each(c4, [&attributes](const auto & node){attributes[node] = 0;});
    std::ranges::for_each(c5, [&attributes](const auto & node){attributes[node] = 0;}); 
    auto dbsc = fishnet::DBSCBuilder<T>()
        .setEps(3.0) // now c1,c2 and c3 are spatially reachable since we set a higher eps, but c3 should still be considered its own cluster since it has a very different attribute value compared to c1 and c2
        .setBeta(2)
        .setMinPts(3) // now c5 should be detected as noise since minPts is 3
        .setDistanceFunction(PointDistanceFunction{})
        .setAttributeExtractor([&attributes](const auto & node){return attributes.at(node);})
        .setT1(2.0) // setting a custom T1 that is higher than the attribute difference between the outlier node and the other nodes in c1
        .build();
    auto result = dbsc(graph);
    EXPECT_SIZE(result.clusters, 3);
    auto c1AndC3 = [this]{
        std::vector<T> nodes;
        nodes.insert(nodes.end(), c1.begin(), c1.end());
        nodes.insert(nodes.end(), c3.begin(), c3.end());
        return nodes;
    }();
    EXPECT_CONTAINS_CLUSTER(result.clusters, c1AndC3); // c1 and c3 should be clustered together since their attribute diff <= custom T1 and spatial distance <= eps
    EXPECT_CONTAINS_CLUSTER(result.clusters, c2);
    EXPECT_CONTAINS_CLUSTER(result.clusters, c4);
    EXPECT_SIZE(result.noise, 2);  // the two nodes of c5
 }