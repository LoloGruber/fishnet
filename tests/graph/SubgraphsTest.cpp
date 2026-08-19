#include <gtest/gtest.h>
#include <fishnet/Graph.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include <fishnet/TestUtil.hpp>
#include "GraphTestUtil.h"
#include "IDNode.h"

using namespace fishnet::graph;
using namespace testutil;

class SubgraphsTest : public ::testing::Test {
protected:
    void SetUp() override {
        // undirected graph with three components:
        // component A: triangle n0-n1-n2-n0
        compA.addEdge(n0, n1);
        compA.addEdge(n1, n2);
        compA.addEdge(n2, n0);

        // component B: single edge n3-n4
        compB.addEdge(n3, n4);

        // component C: isolated node n5
        compC.addNode(n5);

        // build combined graph
        for (auto & n : {n0, n1, n2, n3, n4, n5})
            combinedUndirected.addNode(n);
        combinedUndirected.addEdge(n0, n1);
        combinedUndirected.addEdge(n1, n2);
        combinedUndirected.addEdge(n2, n0);
        combinedUndirected.addEdge(n3, n4);
    }
    // Nodes for undirected graph tests
    IDNode n0;
    IDNode n1;
    IDNode n2;
    IDNode n3;
    IDNode n4;
    IDNode n5;

    UndirectedGraph<IDNode> emptyUndirected;
    UndirectedGraph<IDNode> compA;
    UndirectedGraph<IDNode> compB;
    UndirectedGraph<IDNode> compC;
    UndirectedGraph<IDNode> combinedUndirected;
};

TEST_F(SubgraphsTest, emptyGraph) {
    auto result = BFS::subgraphs(emptyUndirected);
    EXPECT_EMPTY(result);
}

TEST_F(SubgraphsTest, singleComponent) {
    auto result = BFS::subgraphs(compA);
    EXPECT_SIZE(result, 1);
    EXPECT_GRAPH_EQ(result[0], compA);
}

TEST_F(SubgraphsTest, multipleComponentsUndirected) {
    auto result = BFS::subgraphs(combinedUndirected);
    EXPECT_SIZE(result, 3);

    std::vector<UndirectedGraph<IDNode>> expected = {compA, compB, compC};
    EXPECT_GRAPHS_EQ(result, expected);
}

TEST_F(SubgraphsTest, isolatedNodes) {
    UndirectedGraph<IDNode> g;
    g.addNode(n0);
    g.addNode(n1);
    g.addNode(n2);

    auto result = BFS::subgraphs(g);
    EXPECT_SIZE(result, 3);

    for (const auto& subgraph : result) {
        EXPECT_SIZE(subgraph.getNodes(), 1);
        EXPECT_EMPTY(subgraph.getEdges());
    }
}

TEST_F(SubgraphsTest, withCustomProducer) {
    auto producer = []() { return UndirectedGraph<IDNode>(); };
    auto result = BFS::subgraphs(combinedUndirected, producer);
    EXPECT_SIZE(result, 3);

    std::vector<UndirectedGraph<IDNode>> expected = {compA, compB, compC};
    EXPECT_GRAPHS_EQ(result, expected);
}

TEST_F(SubgraphsTest, verifySubgraphIndependence) {
    auto result = BFS::subgraphs(combinedUndirected);
    ASSERT_EQ(result.size(), 3);

    // identify which index corresponds to which component by node count
    size_t idxTriangle = 0;
    for (size_t i = 0; i < result.size(); ++i) {
        if (result[i].getNodes().size() == 3) {
            idxTriangle = i;
            break;
        }
    }
    // modify the triangle subgraph
    IDNode extra;
    result[idxTriangle].addNode(extra);
    result[idxTriangle].addEdge(n0, extra);
    auto compAModified = compA;
    compAModified.addNode(extra);
    compAModified.addEdge(n0, extra);
    std::vector<UndirectedGraph<IDNode>> expected = {compAModified, compB, compC};
    EXPECT_GRAPHS_EQ(result, expected);
}

TEST_F(SubgraphsTest, verifySubgraphContents) {
    auto result = BFS::subgraphs(combinedUndirected);
    ASSERT_EQ(result.size(), 3);

    // find the triangle component (3 nodes, 3 edges)
    bool foundTriangle = false;
    bool foundEdge = false;
    bool foundIsolated = false;

    for (const auto& subgraph : result) {
        auto nodes = subgraph.getNodes();
        auto edges = subgraph.getEdges();
        if (nodes.size() == 3 && edges.size() == 3) {
            foundTriangle = true;
            EXPECT_CONTAINS(nodes, n0);
            EXPECT_CONTAINS(nodes, n1);
            EXPECT_CONTAINS(nodes, n2);
        } else if (nodes.size() == 2 && edges.size() == 1) {
            foundEdge = true;
            EXPECT_CONTAINS(nodes, n3);
            EXPECT_CONTAINS(nodes, n4);
        } else if (nodes.size() == 1 && edges.empty()) {
            foundIsolated = true;
            EXPECT_CONTAINS(nodes, n5);
        }
    }
    EXPECT_TRUE(foundTriangle);
    EXPECT_TRUE(foundEdge);
    EXPECT_TRUE(foundIsolated);
}
