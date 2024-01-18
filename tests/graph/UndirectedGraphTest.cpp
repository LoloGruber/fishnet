#include <gtest/gtest.h>
#include "Graph.h"
#include "IDNode.h"
#include <memory>
#include "Testutil.h"
#include "GraphTestUtil.h"

using UGraph =  fishnet::graph::UndirectedGraph<IDNode>;
using namespace testutil;
using namespace fishnet::graph;

class UndirectedGraphTest: public ::testing::Test {
protected:
    void SetUp()override {
        empty = UGraph();
        simple = getSimpleUndirectedGraph(n1,n2);

        std::vector<IDNode> n = {n0,n1};
        graph = UGraph(n);
        // graph.addNode(n0);
        graph.addEdge(n0,n1);
        graph.addEdge(n0,n2);
        graph.addEdge(n0,n4);
        graph.addEdge(n1,n4);
        graph.addEdge(n2,n4);
        graph.addEdge(n4,n3);

        /**     1----|  /--3
         *      |    | /  
         *      0 -- 4 -- 2
         *      |         | 
         *      |---------|
         * */
    }
    UGraph empty; 
    UGraph simple;
    UGraph graph;
    IDNode n0 = IDNode();
    IDNode n1 = IDNode();
    IDNode n2 = IDNode();
    IDNode n3 = IDNode();
    IDNode n4 = IDNode();
    std::vector<IDNode> nodes = {n0,n1,n2,n3,n4};
};

TEST_F(UndirectedGraphTest, addNode){
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
    EXPECT_EQ(empty.getNodes().size(), 2);
}

TEST_F(UndirectedGraphTest, addNodes){
    auto nodes = getVectorOfNodes(10);
    // empty.addNodes(nodes);
    EXPECT_NO_FATAL_FAILURE(empty.addNodes(nodes)); 
    EXPECT_CONTAINS_ALL(empty.getNodes(),nodes);
}

TEST_F(UndirectedGraphTest, addNodesRValue){
    EXPECT_NO_FATAL_FAILURE(empty.addNodes(getVectorOfNodes(3)));
    EXPECT_EQ(empty.getNodes().size(),3);
}

TEST_F(UndirectedGraphTest, addNodeVarArgs){
    IDNode x;
    IDNode y;
    EXPECT_NO_FATAL_FAILURE(empty.addNode(x,y));
    EXPECT_CONTAINS(empty.getNodes(),x);
    EXPECT_CONTAINS(empty.getNodes(),y);
    EXPECT_SIZE(empty.getNodes(),2);
}

TEST_F(UndirectedGraphTest, containsNode){
    EXPECT_TRUE(simple.containsNode(n1));
    EXPECT_FALSE(empty.containsNode(n1));
    EXPECT_TRUE(graph.containsNode(n1));
    EXPECT_FALSE(simple.containsNode(n0));
    EXPECT_TRUE(graph.containsNode(n0));
}


TEST_F(UndirectedGraphTest, removeNode){
    EXPECT_TRUE(simple.containsNode(n2));
    EXPECT_TRUE(simple.containsNode(n1));
    EXPECT_TRUE(simple.containsEdge(n2,n1));
    EXPECT_NO_FATAL_FAILURE(simple.removeNode(n1));
    EXPECT_TRUE(simple.containsNode(n2));
    EXPECT_FALSE(simple.containsNode(n1));
    auto nodeCollection = simple.getNodes();
    EXPECT_CONTAINS(nodeCollection,n2);
    EXPECT_NOT_CONTAINS(nodeCollection,n1);
    EXPECT_FALSE(simple.containsEdge(n1,n2));
    EXPECT_FALSE(simple.containsEdge(n2,n1));
}

TEST_F(UndirectedGraphTest, getNodes){
    EXPECT_CONTAINS(simple.getNodes(),n1);
    EXPECT_CONTAINS(simple.getNodes(),n2);
    EXPECT_NOT_CONTAINS(simple.getNodes(),n0);
    EXPECT_SIZE(simple.getNodes(),2);
}

TEST_F(UndirectedGraphTest, getNeighbours){
    for(auto& n : nodes) {
        EXPECT_TRUE(graph.containsNode(n));
    }
    auto neighboursOfNodeZero = {n1,n2,n4};
    for(auto & neighbour : neighboursOfNodeZero) {
        EXPECT_CONTAINS(graph.getNeighbours(n0), neighbour);
    }
    EXPECT_NOT_CONTAINS(graph.getNeighbours(n0),n3);
    auto otherNode = IDNode(); //not contained in graph
    EXPECT_EMPTY(graph.getNeighbours(otherNode));
}

TEST_F(UndirectedGraphTest, getReachableFrom){
    std::vector<IDNode> reachableFromNodeFour = {n0,n1,n2,n3};
    EXPECT_CONTAINS_ALL(graph.getReachableFrom(n4), reachableFromNodeFour);
    EXPECT_NOT_CONTAINS(graph.getReachableFrom(n4),n4);
    EXPECT_CONTAINS(graph.getReachableFrom(n3),n4);
    EXPECT_NOT_CONTAINS(graph.getReachableFrom(n3), n0);
    auto notContained = IDNode();
    EXPECT_EQ(graph.getReachableFrom(notContained).size(),0);
}


TEST_F(UndirectedGraphTest, addEdge){
    UGraph g;
    auto from = IDNode();
    auto to = IDNode();
    EXPECT_NO_FATAL_FAILURE(g.addEdge(from, to ));
    EXPECT_TRUE(g.containsNode(from));
    EXPECT_TRUE(g.containsNode(to));
    EXPECT_TRUE(g.containsEdge(from,to));
    EXPECT_TRUE(g.containsEdge(to,from));
    auto edge = UndirectedEdge<IDNode>(from,to);
    EXPECT_CONTAINS(g.getEdges(), edge); 
}

TEST_F(UndirectedGraphTest, containsEdge){
    EXPECT_TRUE(simple.containsNode(n2));
    EXPECT_TRUE(simple.containsNode(n1));
    EXPECT_TRUE(simple.containsEdge(n1,n2));
    EXPECT_TRUE(simple.containsEdge(n2,n1));
    auto edge = UndirectedEdge<IDNode>(n1,n2);
    auto edgeReversed = UndirectedEdge<IDNode>(n2,n1);
    EXPECT_TRUE(simple.containsEdge(edge));
    EXPECT_TRUE(simple.containsEdge(edgeReversed));
    EXPECT_CONTAINS(simple.getEdges(),edge);
    EXPECT_CONTAINS(simple.getEdges(),edgeReversed);
    auto notContainedEdge = UndirectedEdge<IDNode>(n0,n1);
    EXPECT_FALSE(simple.containsEdge(notContainedEdge));
    EXPECT_NOT_CONTAINS(simple.getEdges(),notContainedEdge);
}

TEST_F(UndirectedGraphTest, removeEdgeSimple){
    EXPECT_NO_FATAL_FAILURE(simple.removeEdge(n0,n1));
    EXPECT_TRUE(simple.containsNode(n1));
    EXPECT_NO_FATAL_FAILURE(simple.removeEdge(n1,n2));
    auto edge = UndirectedEdge<IDNode>(n1,n2);
    EXPECT_FALSE(simple.containsEdge(edge));
    EXPECT_TRUE(simple.containsNode(n1));
    EXPECT_TRUE(simple.containsNode(n2));
    EXPECT_NOT_CONTAINS(simple.getEdges(),edge);
}

TEST_F(UndirectedGraphTest, removeEdge){
    EXPECT_NO_FATAL_FAILURE(graph.removeEdge(n3,n4));
    EXPECT_TRUE(graph.containsNode(n3));
    EXPECT_TRUE(graph.containsNode(n4));
    EXPECT_FALSE(graph.containsEdge(n3,n4));
    EXPECT_FALSE(graph.containsEdge(n4,n3));
    EXPECT_EQ(graph.getNeighbours(n3).size(), 0);
    EXPECT_EQ(graph.getOutboundEdges(n3).size(),0);
    EXPECT_NOT_CONTAINS(graph.getNeighbours(n4),n3);
    EXPECT_CONTAINS(graph.getNeighbours(n4),n0);
    EXPECT_CONTAINS(graph.getNeighbours(n4),n1);
    EXPECT_CONTAINS(graph.getNeighbours(n4),n2);
    EXPECT_CONTAINS(graph.getOutboundEdges(n4), UndirectedEdge<IDNode>(n0,n4));
    EXPECT_TRUE(graph.containsEdge(n4,n2));
    EXPECT_TRUE(graph.containsEdge(n4,n0));
    EXPECT_TRUE(graph.containsEdge(n4,n1));
    EXPECT_EQ(graph.getNeighbours(n4).size(),3);
    EXPECT_EQ(graph.getOutboundEdges(n4).size(),3);
}

TEST_F(UndirectedGraphTest, makeEdge){
    auto n1 = IDNode();
    auto n2 = IDNode();
    auto e1 = empty.makeEdge(n1,n2);
    auto e2 = empty.makeEdge(n2,n1);
    //using E = UndirectedEdge<IDNode,int>;
    //EXPECT_NO_FATAL_FAILURE(E e1Casted = dynamic_cast<E &>(e1));
    //EXPECT_NO_FATAL_FAILURE(E e2Casted = dynamic_cast<E&>(e2));
    //EXPECT_BAD_CAST<DirectedEdge<IDNode,int>>(e1);
    EXPECT_TRUE(e1 == e2); // UndirectedEdges
}

TEST_F(UndirectedGraphTest, getOutboundEdges){
    using E = UndirectedEdge<IDNode>;
    auto outboundEdgesOfNodeZero = {
        E(n0,n1),
        E(n0,n2),
        E(n0,n4)
    };
    for(auto const& outboundEdge : outboundEdgesOfNodeZero) {
        EXPECT_CONTAINS(graph.getOutboundEdges(n0),outboundEdge);
    }
    EXPECT_NOT_CONTAINS(graph.getOutboundEdges(n0), E(n0,n3));
    auto from3to4 = E(n3,n4);
    auto reversed = E(n4,n3);
    EXPECT_CONTAINS(graph.getOutboundEdges(n3),from3to4);
    EXPECT_CONTAINS(graph.getOutboundEdges(n3),reversed);
    EXPECT_CONTAINS(graph.getOutboundEdges(n4),from3to4);
    EXPECT_CONTAINS(graph.getOutboundEdges(n4),reversed);
}

TEST_F(UndirectedGraphTest, getInboundEdges){
    using E = UndirectedEdge<IDNode>;
    std::vector<E> inboundEdgesOfNodeZero = std::vector<E>();
    inboundEdgesOfNodeZero.emplace_back(E(n1,n0));
    inboundEdgesOfNodeZero.emplace_back(E(n2,n0));
    inboundEdgesOfNodeZero.emplace_back(E(n4,n0));
    EXPECT_CONTAINS_ALL(graph.getInboundEdges(n0),inboundEdgesOfNodeZero);
    EXPECT_NOT_CONTAINS(graph.getInboundEdges(n0), E(n3,n0));
    auto from4to3 = E(n4,n3);
    auto reversed = E(n3,n4);
    EXPECT_CONTAINS(graph.getOutboundEdges(n3),from4to3);
    EXPECT_CONTAINS(graph.getOutboundEdges(n3),reversed);
    EXPECT_CONTAINS(graph.getOutboundEdges(n4),from4to3);
    EXPECT_CONTAINS(graph.getOutboundEdges(n4),reversed);
}

TEST_F(UndirectedGraphTest, clear){
    simple.clear();
    EXPECT_EMPTY(simple.getNodes());
    EXPECT_EMPTY(simple.getEdges());
}
