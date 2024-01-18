#include <gtest/gtest.h>
#include "WeightedGraph.h"
#include "XYNode.h"
#include "Testutil.h"
#include "GraphTestUtil.h"



struct DistanceFunction{
    double operator()(const XYNode & n1, const XYNode & n2) const {
        return n1.distanceTo(n2);
    }
};
using namespace testutil;
using namespace fishnet::graph;
using WG = Weighted<UndirectedGraph<XYNode>,double,DistanceFunction>;
using DG = GraphDecorator<WG,UndirectedGraph<XYNode>,typename WG::edge_type>;
using AG = AbstractGraph<DG,typename DG::edge_type, typename DG::adj_container_type>;
static_assert(Graph<WG>);
static_assert(std::derived_from<WG,DG>);



class WeightedGraphTest: public ::testing::Test{
protected:
    void SetUp() override {
        graph = WG();
        graph.addNode(n0,n1,n2,n3);
        graph.addEdge(n0,n1);
        graph.addEdge(n1,n2);
        graph.addEdge(n2,n3);
        empty = WG();
    }
    WG graph;
    WG empty;
    XYNode n0 {0,0};
    XYNode n1 {1,1};
    XYNode n2 {0,1};
    XYNode n3 {3,0};

};

static void test_graph_concept(fishnet::graph::Graph auto & g){
    g.getEdges();
}

static void test_weighted_graph_concept(){

}

TEST_F(WeightedGraphTest, concepts){
    test_graph_concept(empty);
}


TEST_F(WeightedGraphTest, addNode){
    fishnet::graph::Weighted<fishnet::graph::UndirectedGraph<XYNode>,double,DistanceFunction> x;
    XYNode myNode {0,0};
    x.addNode(myNode);
    x.addNode(XYNode(1,1));
    EXPECT_CONTAINS(x.getNodes(),myNode);
    EXPECT_CONTAINS(x.getNodes(),XYNode(1,1));
    EXPECT_SIZE(x.getNodes(),2);
}

TEST_F(WeightedGraphTest, addNodes){
    auto nodes = randomNodes(10);
    empty.addNodes(nodes);
    EXPECT_NO_FATAL_FAILURE(empty.addNodes(nodes));
    EXPECT_CONTAINS_ALL(empty.getNodes(),nodes);
    EXPECT_SIZE(empty.getNodes(),10);
}

TEST_F(WeightedGraphTest, addNodesRVal){
    EXPECT_NO_FATAL_FAILURE(empty.addNodes(randomNodes(10)));
    EXPECT_SIZE(empty.getNodes(),10);
}

TEST_F(WeightedGraphTest, addNodeVarArgs){
    XYNode x {0,0};
    XYNode y {1,1};
    EXPECT_NO_FATAL_FAILURE(empty.addNode(x,y));
    EXPECT_CONTAINS(empty.getNodes(),x);
    EXPECT_CONTAINS(empty.getNodes(),y);
    EXPECT_SIZE(empty.getNodes(),2);
}

TEST_F(WeightedGraphTest, containsNode){
    EXPECT_TRUE(graph.containsNode(n0));
    EXPECT_FALSE(graph.containsNode(XYNode{-100000,-10000}));
    EXPECT_FALSE(empty.containsNode(n0));
    EXPECT_TRUE(graph.containsNode(XYNode(0,0)));
    XYNode newNode {42,42};
    empty.addNode(newNode);
    EXPECT_TRUE(empty.containsNode(newNode));
}

TEST_F(WeightedGraphTest, removeNode){
    EXPECT_TRUE(graph.containsNode(n0));
    EXPECT_NO_FATAL_FAILURE(graph.removeNode(n0));
    EXPECT_FALSE(graph.containsNode(n0));
    EXPECT_CONTAINS(graph.getNodes(),n1);
    EXPECT_CONTAINS(graph.getNodes(),n2);
    EXPECT_CONTAINS(graph.getNodes(),n3);
    EXPECT_NOT_CONTAINS(graph.getNodes(),n0);
    EXPECT_NO_FATAL_FAILURE(graph.removeNode(n0));
    EXPECT_NO_FATAL_FAILURE(empty.removeNode(XYNode{-1,-1}));
}

TEST_F(WeightedGraphTest, getNodes){
    std::vector<XYNode> nodes = {n0,n1,n2,n3};
    EXPECT_CONTAINS_ALL(graph.getNodes(),nodes);
    EXPECT_NOT_CONTAINS(graph.getNodes(),XYNode{-1,-1});
    EXPECT_SIZE(graph.getNodes(),4);
    EXPECT_EMPTY(empty.getNodes());
}

TEST_F(WeightedGraphTest, getNeighbours){
    std::vector<XYNode> neighboursOfn1 = {n0,n2};
    EXPECT_CONTAINS_ALL(graph.getNeighbours(n1),neighboursOfn1);
    EXPECT_SIZE(graph.getNeighbours(n1),2);
    EXPECT_EMPTY(empty.getNeighbours(n0));
    EXPECT_EMPTY(graph.getNeighbours(XYNode{-1,-1}));
}

TEST_F(WeightedGraphTest, getReachableFrom){
    std::vector<XYNode> reachableFromn2 = {n1,n3};
    EXPECT_CONTAINS_ALL(graph.getReachableFrom(n2),reachableFromn2);
    EXPECT_NOT_CONTAINS(graph.getReachableFrom(n2), n0);
    EXPECT_EMPTY(graph.getReachableFrom(XYNode{-1,-1}));
    EXPECT_EMPTY(empty.getReachableFrom(n0));
}

TEST_F(WeightedGraphTest, addEdge){
    EXPECT_NO_FATAL_FAILURE(graph.addEdge(n3,n0));
    EXPECT_TRUE(graph.containsEdge(n3,n0));
    XYNode x {-1,-1};
    XYNode y {-2,-2};
    graph.addEdge(x,y);
    EXPECT_TRUE(graph.containsNode(x));
    EXPECT_TRUE(graph.containsNode(y));
    EXPECT_TRUE(graph.containsEdge(x,y));
    XYNode z {-3,-3};
    XYNode w {-4,-4};
    auto newEdge = graph.makeEdge(z,w);
    EXPECT_NO_FATAL_FAILURE(newEdge.getWeight());
    EXPECT_NO_FATAL_FAILURE(graph.addEdge(newEdge));
    EXPECT_TRUE(graph.containsEdge(newEdge));
}

TEST_F(WeightedGraphTest, addEdgeWeighted){
    EXPECT_NO_FATAL_FAILURE(empty.addEdge(n3,n0,42.0));
    auto e1 = graph.makeEdge(n3,n0,42.0);
    EXPECT_NO_FATAL_FAILURE(empty.addEdge(XYNode{-2,-1},XYNode{-2,-2}));
    auto e2 = graph.makeEdge(XYNode{-2,-1},XYNode{-2,-2},1);
    auto e3 = graph.makeEdge(n0,n1,100.0);
    EXPECT_NO_FATAL_FAILURE(empty.addEdge(e3));
    std::vector<typename WG::edge_type> toBeContained = {e1,e2,e3};
    EXPECT_CONTAINS_ALL(empty.getEdges(),toBeContained);
    EXPECT_SIZE(empty.getEdges(),toBeContained.size());
}

TEST_F(WeightedGraphTest, containsEdge ){
    EXPECT_TRUE(graph.containsEdge(n0,n1));
    EXPECT_TRUE(graph.containsEdge(n1,n2,1));
    EXPECT_FALSE(graph.containsEdge(n0,n1,1)); //different annotation -> false
    auto fromN1toN2 = graph.makeEdge(n1,n2,1.0);
    EXPECT_TRUE(graph.containsEdge(fromN1toN2));
    static_assert(!WG::edge_type::isDirected());
    auto fromN2toN1 = graph.makeEdge(n1,n2,1.0); // underlying graph is undirected
    EXPECT_TRUE(graph.containsEdge(fromN2toN1));
}

TEST_F(WeightedGraphTest, makeWeightedEdge){
    XYNode from {1,1};
    XYNode to {0,0};
    auto expectedEdge = WeightEdge<fishnet::graph::UndirectedEdge<XYNode>,double,DistanceFunction>(from,to);
    EXPECT_EQ(WG::makeWeightedEdge(from,to),expectedEdge);
    double individualWeight = 100.0;
    auto expectedIndividualEdge = WeightEdge<fishnet::graph::UndirectedEdge<XYNode>,double,DistanceFunction>(from,to,individualWeight);
    EXPECT_EQ(WG::makeWeightedEdge(from,to,individualWeight),expectedIndividualEdge);
}

TEST_F(WeightedGraphTest, removeEdge){
    EXPECT_TRUE(graph.containsEdge(n0,n1));
    EXPECT_NO_FATAL_FAILURE(graph.removeEdge(n0,n1));
    EXPECT_FALSE(graph.containsEdge(n0,n1));
    EXPECT_TRUE(graph.containsNode(n0));
    EXPECT_TRUE(graph.containsNode(n1));
    EXPECT_EMPTY(graph.getNeighbours(n0));
    EXPECT_SIZE(graph.getNeighbours(n1),1);
    auto fromN1toN2 = graph.makeEdge(n1,n2);
    EXPECT_TRUE(graph.containsEdge(n1,n2));
    EXPECT_NO_FATAL_FAILURE(graph.removeEdge(fromN1toN2));
    EXPECT_FALSE(graph.containsEdge(n1,n2));
    EXPECT_TRUE(graph.containsNode(n2));
    EXPECT_TRUE(graph.containsNode(n1));
    auto fromN2toN3DifferentWeight = graph.makeEdge(n2,n3,-1.0);
    EXPECT_TRUE(graph.containsEdge(n2,n3));
    EXPECT_NO_FATAL_FAILURE(graph.removeEdge(fromN2toN3DifferentWeight));
    EXPECT_TRUE(graph.containsEdge(n2,n3)); // edge should not be removed, since weights are different
    EXPECT_TRUE(graph.containsNode(n2));
    EXPECT_TRUE(graph.containsNode(n3));
    EXPECT_SIZE(graph.getEdges(),1); // only n2 <-> n3 left
}

TEST_F(WeightedGraphTest, getOutboundEdges){
    std::vector<typename WG::edge_type> outboundN1 = {
        graph.makeEdge(n1,n0), graph.makeEdge(n1,n2)
    };
    EXPECT_CONTAINS_ALL(graph.getOutboundEdges(n1),outboundN1);
    EXPECT_NOT_CONTAINS(graph.getOutboundEdges(n1),graph.makeEdge(n2,n3));
    EXPECT_EMPTY(empty.getOutboundEdges(n0));
    for(auto & e: graph.getOutboundEdges(n1)) {
        EXPECT_NO_FATAL_FAILURE(e.getWeight());
    }
}

TEST_F(WeightedGraphTest, getInboundEdges){
    std::vector<typename WG::edge_type> inboundN1 = {
        graph.makeEdge(n0,n1), graph.makeEdge(n2,n1)
    };
    EXPECT_CONTAINS_ALL(graph.getInboundEdges(n1),inboundN1);
    EXPECT_NOT_CONTAINS(graph.getInboundEdges(n1),graph.makeEdge(n2,n3));
    EXPECT_SIZE(graph.getInboundEdges(n1),2);
    EXPECT_EMPTY(empty.getInboundEdges(n0));
    for(auto & e: graph.getInboundEdges(n1)) {
        EXPECT_NO_FATAL_FAILURE(e.getWeight());
    }
}

TEST_F(WeightedGraphTest, clear){
    EXPECT_NO_FATAL_FAILURE(graph.clear());
    EXPECT_EMPTY(graph.getNodes());
    EXPECT_EMPTY(graph.getEdges());
    EXPECT_EMPTY(graph.getNeighbours(n1));
    EXPECT_EMPTY(graph.getReachableFrom(n1));
    EXPECT_EMPTY(graph.getInboundEdges(n1));
    EXPECT_EMPTY(graph.getOutboundEdges(n1));
    double weight = 100.0;
    auto edgeWithSpecificWeight = empty.makeEdge(XYNode{0,0},XYNode{0,0},weight); //testing self edges as well
    empty.addEdge(edgeWithSpecificWeight);
    auto actualEdge = *(empty.getEdges().begin());
    EXPECT_EQ(actualEdge,edgeWithSpecificWeight);
    EXPECT_EQ(actualEdge.getWeight(),weight);
    EXPECT_EQ(empty.makeEdge(XYNode{0,0},XYNode{0,0}).getWeight(),weight);
    empty.clear();
    EXPECT_EMPTY(empty.getEdges());
    EXPECT_NE(empty.makeEdge(XYNode{0,0},XYNode{0,0}).getWeight(),weight);
}