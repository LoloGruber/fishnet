#include <gtest/gtest.h>
#include "Testutil.h"
#include <fishnet/Graph.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include <fishnet/DFSAlgorithm.hpp>
#include "DataIDNode.h"


using namespace fishnet::graph;
using namespace testutil;
using UGraph = fishnet::graph::UndirectedGraph<DataIDNode<std::string>>;
using DGraph = fishnet::graph::DirectedGraph<DataIDNode<std::string>>;

static_assert(fishnet::util::Printable<DataIDNode<std::string>>);
static_assert(fishnet::util::Printable<UGraph::edge_type>);

class SearchPathTest : public ::testing::Test{
protected:
    void SetUp() override {
        g.addEdge(n1,n2);
        g.addEdge(n1,n3);
        g.addEdge(n2,n4);
        g.addEdge(n3,n5);
        g.addEdge(n4,n6);
        g.addEdge(n5,n7);
        g.addEdge(n7,n6);
        g.addEdge(n4,n5);
        
        g.addEdge(n8,n9);

        dg.addEdge(n1,n2);
        dg.addEdge(n2,n4);
        dg.addEdge(n4,n3);
        dg.addEdge(n3,n2);
        dg.addEdge(n2,n1);
        dg.addEdge(n1,n5);
        dg.addEdge(n5,n6);
        dg.addEdge(n6,n4);

        dg.addEdge(n6,n7);
    }


    DataIDNode<std::string> n1{"n1"};
    DataIDNode<std::string> n2{"n2"};
    DataIDNode<std::string> n3{"n3"};
    DataIDNode<std::string> n4{"n4"};
    DataIDNode<std::string> n5{"n5"};
    DataIDNode<std::string> n6{"n6"};
    DataIDNode<std::string> n7{"n7"};
    DataIDNode<std::string> n8{"n8"}; //isolated
    DataIDNode<std::string> n9{"n9"}; //isolated

    UGraph g;
    DGraph dg;
};

TEST_F(SearchPathTest, ComplexPath){
    auto path = BFS::getPath(g,n1,n6);
    std::vector<UGraph::edge_type> expected;
    expected.push_back(g.makeEdge(n1,n2));
    expected.push_back(g.makeEdge(n2,n4));
    expected.push_back(g.makeEdge(n4,n6));
    EXPECT_EQ(expected,path); 
}

TEST_F(SearchPathTest, NoPath) {
    auto path = BFS::getPath(g,n6,n8);
    EXPECT_EQ(path.size(),0);
}

TEST_F(SearchPathTest, SimplePath) {
    auto path = BFS::getPath(g,n8,n9);
    std::vector<typename UndirectedGraph<DataIDNode<std::string>>::edge_type> expected;
    expected.push_back(g.makeEdge(n9,n8)); //undirected graph => (n8,n9) == (n9,n8)
    EXPECT_EQ(expected,path);
}


TEST_F(SearchPathTest, DirectedSimplePath){
    auto actual = BFS::getPath(dg,n1,n2);
    std::vector<typename DirectedGraph<DataIDNode<std::string>>::edge_type> expected;
    expected.push_back(dg.makeEdge(n1,n2));
    EXPECT_EQ(expected,actual);
}

TEST_F(SearchPathTest, DirectedSimplePathReversed){
    auto actual = BFS::getPath(dg,n2,n1);
    std::vector<typename DirectedGraph<DataIDNode<std::string>>::edge_type> expected;
    expected.push_back(dg.makeEdge(n2,n1));
    EXPECT_EQ(expected,actual);
}

TEST_F(SearchPathTest, NoPathInvalidSource){
    auto path = BFS::getPath(dg,n8,n7); //Source invalid
    EXPECT_TRUE(path.empty());
}

TEST_F(SearchPathTest, NoPathInvalidGoal){
    auto path = BFS::getPath(dg,n1,n9); //Goal invalid
    EXPECT_TRUE(path.empty());
}

TEST_F(SearchPathTest, NoPathDirectedDueToOrientation){
    auto path = BFS::getPath(dg,n7,n6);
    EXPECT_TRUE(path.empty());
}

TEST_F(SearchPathTest, DirectedComplexPath) {
    auto actual = BFS::getPath(dg,n1,n3);
    typename std::vector<DGraph::edge_type> expected;
    expected.push_back(dg.makeEdge(n1,n2));    
    expected.push_back(dg.makeEdge(n2,n4));
    expected.push_back(dg.makeEdge(n4,n3));
    EXPECT_EQ(expected,actual);
}

TEST_F(SearchPathTest, DFSSimpleUndirected){
    auto path = DFS::getPath(g,n8,n9);
    std::vector<UGraph::edge_type> expected;
    expected.push_back(g.makeEdge(n8,n9));
    EXPECT_EQ(expected,path);
}

TEST_F(SearchPathTest, DFSComplexPath){
    auto path = DFS::getPath(g,n1,n6);
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(path.front().getFrom(),n1);
    EXPECT_EQ(path.back().getTo(),n6);
    for(const auto & e: path) {
        EXPECT_TRUE(g.containsEdge(e));
    }
}

