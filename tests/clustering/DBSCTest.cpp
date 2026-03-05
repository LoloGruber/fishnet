#include <fishnet/TestUtil.hpp>
#include <fishnet/Vec2D.hpp>
#include <fishnet/DBSC.hpp>
#include "ClusteringTestUtil.hpp"

using T = fishnet::geometry::Vec2DStd;
using G = fishnet::graph::UndirectedGraph<T>;

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
    auto distanceFunction = [](const auto & lhs, const auto & rhs){return lhs.distance(rhs);};
    fishnet::DBSC<T> dbsc(2, distanceFunction);
    auto result = dbsc.cluster(graph);
 }