#include <gtest/gtest.h>
#include <fishnet/Graph.hpp>
#include "IDNode.h"
#include "Testutil.h"
#include "GraphTestUtil.h"

using DGraph = fishnet::graph::DirectedGraph<IDNode>;
using DEdge = DGraph::edge_type;
using namespace testutil;
using namespace fishnet::graph;

class DirectedGraphTest: public ::testing::Test{
protected:
    void SetUp() override {
        simple = getSimpleDirectedGraph(n1,n2);
        graph = DGraph(nodes);
        graph.addEdge(n0,n1);
        graph.addEdge(n0,n2);
        graph.addEdge(n0,n4);
        graph.addEdge(n4,n0);
        graph.addEdge(n1,n4);
        graph.addEdge(n2,n4);
        graph.addEdge(n4,n3);

        /**
         *      1-----|   /->3
         *      ^     |  /    
         *      |     V /  
         *      0 <-> 4 <- 2
         *      |          ^ 
         *      |----------|
         * */

    
    }

    DGraph empty;
    DGraph simple;
    DGraph graph;

    IDNode n0 = IDNode();
    IDNode n1 = IDNode();
    IDNode n2 = IDNode();
    IDNode n3 = IDNode();
    IDNode n4 = IDNode();
    std::vector<IDNode> nodes = {n0,n1,n2,n3,n4};
};

TEST_F(DirectedGraphTest, addNode){
    EXPECT_NO_FATAL_FAILURE(empty.addNode(n3));
    EXPECT_NO_FATAL_FAILURE(empty.addNode(n4));
    EXPECT_TRUE(empty.containsNode(n3));
    EXPECT_TRUE(empty.containsNode(n4));
    EXPECT_FALSE(empty.containsNode(n0));
    auto nodeCollection = empty.getNodes();
    EXPECT_CONTAINS(nodeCollection,n3);
    EXPECT_CONTAINS(nodeCollection,n4);
    EXPECT_NO_FATAL_FAILURE(empty.addNode(n3));
    EXPECT_TRUE(empty.containsNode(n3));
    EXPECT_SIZE(empty.getNodes(), 2);
}

TEST_F(DirectedGraphTest, containsNode){
    EXPECT_TRUE(simple.containsNode(n1));
    EXPECT_FALSE(empty.containsNode(n1));
    EXPECT_TRUE(graph.containsNode(n1));
    EXPECT_FALSE(simple.containsNode(n0));
    EXPECT_TRUE(graph.containsNode(n0));
}

TEST_F(DirectedGraphTest, removeNode){
    EXPECT_TRUE(simple.containsNode(n2));
    EXPECT_TRUE(simple.containsNode(n1));
    EXPECT_TRUE(simple.containsEdge(n1,n2));
    EXPECT_NO_FATAL_FAILURE(simple.removeNode(n1));
    EXPECT_TRUE(simple.containsNode(n2));
    EXPECT_FALSE(simple.containsNode(n1));
    auto nodeCollection = simple.getNodes();
    EXPECT_CONTAINS(nodeCollection,n2);
    EXPECT_NOT_CONTAINS(nodeCollection,n1);
    EXPECT_FALSE(simple.containsEdge(n1,n2));
    EXPECT_FALSE(simple.containsEdge(n2,n1));
}

TEST_F(DirectedGraphTest, getNodes){
    EXPECT_CONTAINS(simple.getNodes(),n1);
    EXPECT_CONTAINS(simple.getNodes(),n2);
    EXPECT_NOT_CONTAINS(simple.getNodes(),n0);
    EXPECT_SIZE(simple.getNodes(),2);
}

TEST_F(DirectedGraphTest, getNeighbours){
    auto neighboursOfNodeFour = {n0,n3};
    for(auto & neighbour : neighboursOfNodeFour) {
        EXPECT_CONTAINS(graph.getNeighbours(n4), neighbour);
    }
    EXPECT_NOT_CONTAINS(graph.getNeighbours(n4), n2);
    EXPECT_NOT_CONTAINS(graph.getNeighbours(n4), n1);
    EXPECT_NOT_CONTAINS(graph.getNeighbours(n4), n4);
    auto neighboursOfNodeZero = {n1,n2,n4};
    for(auto & neighbour : neighboursOfNodeZero){
        EXPECT_CONTAINS(graph.getNeighbours(n0), neighbour);
    }
    auto otherNode = IDNode();
    EXPECT_NOT_CONTAINS(graph.getNeighbours(n0),otherNode);
    EXPECT_EMPTY(graph.getNeighbours(otherNode));
}

TEST_F(DirectedGraphTest, getReachableFrom){
    std::vector<IDNode> reachableFromNodeFour = {n0,n1,n2};
    EXPECT_CONTAINS_ALL(graph.getReachableFrom(n4),reachableFromNodeFour);
    EXPECT_NOT_CONTAINS(graph.getReachableFrom(n4),n3);
    EXPECT_NOT_CONTAINS(graph.getReachableFrom(n4),n4);
    std::vector<IDNode> reachableFromNodeZero =  {n4};
    EXPECT_CONTAINS_ALL(graph.getReachableFrom(n0),reachableFromNodeZero);
    auto other = IDNode();
    EXPECT_NOT_CONTAINS(graph.getReachableFrom(n4),other);
    EXPECT_EMPTY(graph.getReachableFrom(other));
}

TEST_F(DirectedGraphTest, addEdge){
    DGraph g;
    auto from = IDNode();
    auto to = IDNode();
    EXPECT_EMPTY(g.getEdges());
    EXPECT_NO_FATAL_FAILURE(g.addEdge(from,to));
    EXPECT_TRUE(g.containsNode(from));
    EXPECT_TRUE(g.containsNode(to));
    EXPECT_TRUE(g.containsEdge(from,to));
    EXPECT_FALSE(g.containsEdge(to,from));
    auto edge = DEdge(from,to);
    EXPECT_CONTAINS(g.getEdges(),edge);
    EXPECT_SIZE(g.getEdges(),1);
    EXPECT_TRUE(g.containsEdge(edge));
}

TEST_F(DirectedGraphTest, containsEdge){
    EXPECT_TRUE(simple.containsNode(n2));
    EXPECT_TRUE(simple.containsNode(n1));
    EXPECT_TRUE(simple.containsEdge(n1,n2));
    EXPECT_FALSE(simple.containsEdge(n2,n1));
    auto edge = DEdge(n1,n2);
    auto reversed = DEdge(n2,n1);
    EXPECT_TRUE(simple.containsEdge(edge));
    EXPECT_FALSE(simple.containsEdge(reversed));
    auto notContained = DEdge(n1,n0);
    EXPECT_FALSE(simple.containsEdge(notContained));
    EXPECT_NOT_CONTAINS(simple.getEdges(),notContained);
}

TEST_F(DirectedGraphTest, removeEdgeSimple){
    EXPECT_NO_FATAL_FAILURE(simple.removeEdge(n0,n1));
    EXPECT_TRUE(simple.containsNode(n1));
    EXPECT_NO_FATAL_FAILURE(simple.removeEdge(n1,n2));
    auto edge = DEdge(n1,n2);
    EXPECT_FALSE(simple.containsEdge(edge));
    EXPECT_FALSE(simple.containsEdge(n1,n2));
    EXPECT_TRUE(simple.containsNode(n1));
    EXPECT_TRUE(simple.containsNode(n2));
    EXPECT_NOT_CONTAINS(simple.getEdges(),edge);
}

TEST_F(DirectedGraphTest,removeEdge){
    EXPECT_SIZE(graph.getNeighbours(n4),2);
    EXPECT_NO_FATAL_FAILURE(graph.removeEdge(n4,n0));
    EXPECT_TRUE(graph.containsNode(n4));
    EXPECT_TRUE(graph.containsNode(n0));
    EXPECT_FALSE(graph.containsEdge(n4,n0));
    EXPECT_TRUE(graph.containsEdge(n0,n4));
    EXPECT_EMPTY(graph.getReachableFrom(n0));
    EXPECT_SIZE(graph.getNeighbours(n4),1);
    EXPECT_SIZE(graph.getInboundEdges(n0),0);
    EXPECT_SIZE(graph.getOutboundEdges(n0),3);
    EXPECT_SIZE(graph.getInboundEdges(n4),3);
    EXPECT_SIZE(graph.getOutboundEdges(n4),1);
    EXPECT_CONTAINS(graph.getNeighbours(n4),n3);
    EXPECT_NOT_CONTAINS(graph.getNeighbours(n4),n0);
    auto from4to3 = DEdge(n4,n3);
    auto from4to0 = DEdge(n4,n0);
    auto from0to4 = DEdge(n0,n4);
    EXPECT_CONTAINS(graph.getOutboundEdges(n4),from4to3);
    EXPECT_NOT_CONTAINS(graph.getOutboundEdges(n4),from4to0);
    EXPECT_CONTAINS(graph.getOutboundEdges(n0),from0to4);
    EXPECT_NOT_CONTAINS(graph.getInboundEdges(n0), from4to0);
    EXPECT_NOT_CONTAINS(graph.getEdges(), from4to0);
    EXPECT_CONTAINS(graph.getEdges(),from0to4);

}

TEST_F(DirectedGraphTest,makeEdge){
    auto n1 = IDNode();
    auto n2 = IDNode();
    auto e1 = empty.makeEdge(n1,n2);
    auto e2 = empty.makeEdge(n2,n1);
    //EXPECT_NO_FATAL_FAILURE(E e1Casted = dynamic_cast<E &>(e1));
    //EXPECT_NO_FATAL_FAILURE(E e2Casted = dynamic_cast<E &>(e2));
    //EXPECT_BAD_CAST<UndirectedEdge<IDNode,int>,E>(e1);
    EXPECT_FALSE(e1 == e2);
}

TEST_F(DirectedGraphTest, getOutboundEdges){
    using E = DirectedEdge<IDNode>;
    std::vector<E> outboundEdgesOfNodeZero = std::vector<E>();
    outboundEdgesOfNodeZero.emplace_back(E(n0,n1));
    outboundEdgesOfNodeZero.emplace_back(E(n0,n1)); // duplicate should not be a problem
    outboundEdgesOfNodeZero.emplace_back(E(n0,n2));
    outboundEdgesOfNodeZero.emplace_back(E(n0,n4));
    EXPECT_CONTAINS_ALL(graph.getOutboundEdges(n0),outboundEdgesOfNodeZero);
    EXPECT_NOT_CONTAINS(graph.getOutboundEdges(n0), E(n0,n3));
    std::vector<E> outboundEdgesOfNodeFour =  std::vector<E>();
    outboundEdgesOfNodeFour.emplace_back(E(n4,n0));
    outboundEdgesOfNodeFour.emplace_back(E(n4,n3));    
    EXPECT_CONTAINS_ALL(graph.getOutboundEdges(n4),outboundEdgesOfNodeFour);
    EXPECT_NOT_CONTAINS(graph.getOutboundEdges(n4),E(n4,n2));
    EXPECT_NOT_CONTAINS(graph.getOutboundEdges(n4),E(n4,n1));
    EXPECT_EMPTY(graph.getOutboundEdges(n3));
}

TEST_F(DirectedGraphTest,getInboundEdges){
    using E = DirectedEdge<IDNode>;
    EXPECT_CONTAINS(graph.getInboundEdges(n0),E(n4,n0));
    EXPECT_NOT_CONTAINS(graph.getInboundEdges(n0), E(n1,n0));
    EXPECT_NOT_CONTAINS(graph.getInboundEdges(n0), E(n0,n4));
    std::vector<E> inboundEdgesOfNodeFour =  std::vector<E>();
    inboundEdgesOfNodeFour.emplace_back(E(n0,n4));
    inboundEdgesOfNodeFour.emplace_back(E(n1,n4));
    inboundEdgesOfNodeFour.emplace_back(E(n2,n4));    
    EXPECT_CONTAINS_ALL(graph.getInboundEdges(n4),inboundEdgesOfNodeFour);
    EXPECT_NOT_CONTAINS(graph.getInboundEdges(n4),E(n3,n4));
    EXPECT_CONTAINS(graph.getInboundEdges(n3),E(n4,n3));
    EXPECT_SIZE(graph.getInboundEdges(n4),3);
}








