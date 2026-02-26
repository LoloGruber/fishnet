#include <fishnet/TestUtil.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include <fishnet/Graph.hpp>
#include <ranges>
#include "DataIDNode.h"
#include "MoveOnlyAdjacency.hpp"

using namespace fishnet::graph;
using namespace testutil;
using N = DataIDNode<std::string>;
using G = UndirectedGraph<N>;
using E = typename G::edge_type;

class NeighborhoodTest : public ::testing::Test{
protected:
    void SetUp() override {
        g = fishnet::graph::GraphFactory::UndirectedGraph<N>();
        // Neighborhood of n0 with order 1: n0, n10, n11, n12
        g.addEdge(s,order1[0]);
        g.addEdge(s,order1[1]);
        g.addEdge(s,order1[2]);
        // Neighborhood of n0 with order 2: n0, n10, n11, n12, n20, n21, n22, n23
        g.addEdge(order1[0],order2[0]);
        g.addEdge(order1[1],order2[1]);
        g.addEdge(order1[2],order2[2]);
        g.addEdge(order1[2],order2[3]);

        // edge between nodes of k order should not be in k-order neighborhood but should be in k+1 order neighborhood
        g.addEdge(order2[0],order2[1]); 

        g.addEdge(order2[0],order3[0]);
        g.addEdge(order2[1],order3[1]);
        g.addEdge(order2[3],order3[0]);

       // edge between nodes of k order should not be in k-order neighborhood but should be in k+1 order neighborhood
        g.addEdge(order3[0],order3[1]); 

        // not reachable from s
        g.addEdge(notReachable[0],notReachable[1]);
        // isolated node
        g.addNode(z); 
    }


    N s{"s"};
    N x {"x"};
    N y {"y"};
    N z {"z"};
    std::vector<N> order1{N("n10"),N("n11"),N("n12")};
    std::vector<N> order2{N("n20"),N("n21"),N("n22"),N("n23")};
    std::vector<N> order3{N("n30"),N("n31")};
    std::vector<N> notReachable{x,y};
    G g;
};

TEST_F(NeighborhoodTest, order0){
    auto neighborhood = BFS::neighborhood(g,s,0);
    EXPECT_TRUE(neighborhood.containsNode(s));
    EXPECT_SIZE(neighborhood.getNodes(),1);
    EXPECT_EMPTY(neighborhood.getEdges());
}

TEST_F(NeighborhoodTest, order1){
    auto neighborhood = BFS::neighborhood(g,s,1);
    EXPECT_CONTAINS(neighborhood.getNodes(),s);
    EXPECT_CONTAINS_ALL(neighborhood.getNodes(),order1);
    EXPECT_SIZE(neighborhood.getNodes(),4);
    for (const auto & n: order1){
        EXPECT_TRUE(neighborhood.containsEdge(s,n));
    }
    EXPECT_SIZE(neighborhood.getEdges(),3);
}

TEST_F(NeighborhoodTest, order2){
    auto neighborhood = BFS::neighborhood(g,s,2);
    EXPECT_CONTAINS(neighborhood.getNodes(),s);
    EXPECT_CONTAINS_ALL(neighborhood.getNodes(),order1);
    EXPECT_CONTAINS_ALL(neighborhood.getNodes(),order2);
    EXPECT_SIZE(neighborhood.getNodes(),8);
    std::vector<E> expectedEdges = {
        {s,order1[0]},
        {s,order1[1]},
        {s,order1[2]},
        {order1[0],order2[0]},
        {order1[1],order2[1]},
        {order1[2],order2[2]},
        {order1[2],order2[3]},
    };
    EXPECT_UNSORTED_RANGE_EQ(neighborhood.getEdges(),expectedEdges);
}

TEST_F(NeighborhoodTest, order3){
    auto neighborhood = BFS::neighborhood(g,s,3);
    EXPECT_CONTAINS(neighborhood.getNodes(),s);
    EXPECT_CONTAINS_ALL(neighborhood.getNodes(),order1);
    EXPECT_CONTAINS_ALL(neighborhood.getNodes(),order2);
    EXPECT_CONTAINS_ALL(neighborhood.getNodes(),order3);
    EXPECT_SIZE(neighborhood.getNodes(),10);
    std::vector<E> expectedEdges = {
        {s,order1[0]},
        {s,order1[1]},
        {s,order1[2]},
        {order1[0],order2[0]},
        {order1[1],order2[1]},
        {order1[2],order2[2]},
        {order1[2],order2[3]},
        {order2[0],order2[1]},
        {order2[0],order3[0]},
        {order2[1],order3[1]},
        {order2[3],order3[0]}
    };
    EXPECT_UNSORTED_RANGE_EQ(neighborhood.getEdges(),expectedEdges);
}

TEST_F(NeighborhoodTest, orderMax){
    auto neighborhood = BFS::neighborhood(g,s,std::numeric_limits<size_t>::max());
    EXPECT_CONTAINS(neighborhood.getNodes(),s);
    EXPECT_CONTAINS_ALL(neighborhood.getNodes(),order1);
    EXPECT_CONTAINS_ALL(neighborhood.getNodes(),order2);
    EXPECT_CONTAINS_ALL(neighborhood.getNodes(),order3);
    std::vector<E> expectedEdges = {
        {s,order1[0]},
        {s,order1[1]},
        {s,order1[2]},
        {order1[0],order2[0]},
        {order1[1],order2[1]},
        {order1[2],order2[2]},
        {order1[2],order2[3]},
        {order2[0],order2[1]},
        {order2[0],order3[0]},
        {order2[1],order3[1]},
        {order2[3],order3[0]},
        {order3[0],order3[1]}
    };
    EXPECT_UNSORTED_RANGE_EQ(neighborhood.getEdges(),expectedEdges);
    EXPECT_FALSE(neighborhood.containsNode(notReachable[0]));
    EXPECT_FALSE(neighborhood.containsNode(notReachable[1]));
    EXPECT_FALSE(neighborhood.containsNode(z));
}

TEST_F(NeighborhoodTest, isolated){
    auto neighborhood = BFS::neighborhood(g,z,1);
    EXPECT_UNSORTED_RANGE_EQ(neighborhood.getNodes(), std::views::single(z));
    EXPECT_EMPTY(neighborhood.getEdges());
}

TEST_F(NeighborhoodTest, notReachable){
    auto neighborhood = BFS::neighborhood(g,x,5);
    EXPECT_UNSORTED_RANGE_EQ(neighborhood.getNodes(), std::vector<N>{x,y});
    EXPECT_UNSORTED_RANGE_EQ(neighborhood.getEdges(), std::vector<E>{{x,y}});
}

TEST_F(NeighborhoodTest, moveOnlyGraph){
    auto moveOnlyGraph = fishnet::graph::GraphFactory::UndirectedGraph(MoveOnlyAdjacency<N>());
    static_assert(std::is_copy_constructible_v<G>);
    static_assert(!std::is_copy_constructible_v<decltype(moveOnlyGraph)>);
    static_assert(std::is_constructible_v<MoveOnlyAdjacency<N>>);
    moveOnlyGraph.addEdge(s,order1[0]);
    moveOnlyGraph.addEdge(order1[0],order2[0]);
    // this should always work, even if MoveOnlyAdjacency is not copyable, since we provide an output graph
    auto neighborhood = BFS::neighborhood(moveOnlyGraph,s,1,GraphFactory::UndirectedGraph(MoveOnlyAdjacency<N>()));
    // MoveOnlyAdjacency is not copyable but default constructible, so this works
    auto neighborhood2 = BFS::neighborhood(moveOnlyGraph,s,2); 
}

TEST_F(NeighborhoodTest, moveOnlyNoDefaultConstructorGraph){
    auto moveOnlyNoDefConsGraph = fishnet::graph::GraphFactory::UndirectedGraph(MoveOnlyNoDefaultConstructorAdjacency<N>(42));
    static_assert(!std::is_copy_constructible_v<decltype(moveOnlyNoDefConsGraph)>);
    static_assert(!std::is_default_constructible_v<MoveOnlyNoDefaultConstructorAdjacency<N>>);
    moveOnlyNoDefConsGraph.addEdge(s,order1[0]);
    moveOnlyNoDefConsGraph.addEdge(order1[0],order2[0]);
    // this should always work, even if MoveOnlyNoDefaultConstructorAdjacency is not copyable and not default constructible, since we provide an output graph
    auto neighborhood = BFS::neighborhood(moveOnlyNoDefConsGraph,s,1,GraphFactory::UndirectedGraph(MoveOnlyNoDefaultConstructorAdjacency<N>(42)));
    // MoveOnlyNoDefaultConstructorAdjacency is not copyable and not default constructible therefore this should not compile:
    // auto notCompilable = BFS::getNeighborhood(moveOnlyNoDefConsGraph,s,2);
}

