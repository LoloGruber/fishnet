#include <gtest/gtest.h>
#include <fishnet/BinaryFileAdjacency.hpp>
#include <fishnet/Graph.hpp>
#include <fishnet/AdjacencyMap.hpp>
#include <fishnet/TemporaryDirectiory.h>
#include "XYNode.h"
#include <fishnet/TestUtil.hpp>
#include <cstring>

using namespace fishnet::graph;
using namespace testutil;

struct XYNodeToBinary{
    static std::vector<uint8_t> operator()(const XYNode & node) {
        std::vector<uint8_t> data(sizeof(double)*2);
        const double x = node.getX();
        const double y = node.getY();
        std::memcpy(data.data(), &x, sizeof(double));
        std::memcpy(data.data() + sizeof(double), &y, sizeof(double));
        return data;
    }
};

struct XYNodeFromBinary{
    static XYNode operator()(const std::vector<uint8_t> & data) {
        double x, y;
        std::memcpy(&x, data.data(), sizeof(double));
        std::memcpy(&y, data.data() + sizeof(double), sizeof(double));
        return XYNode{x, y};
    }
};

class BinaryFileAdjacencyTest : public ::testing::Test {
protected:
    void SetUp() override { 
        filePath = temporaryDirectory / std::filesystem::path("test_graph.bin");
        const uint64_t NODE_COUNT = 3;
        const uint64_t EDGE_COUNT = 3;
        std::ofstream file(filePath, std::ios::binary);
        // Write nodes
        file.write(reinterpret_cast<const char*>(&NODE_COUNT), sizeof(NODE_COUNT));
        u_int64_t nodeID = 0;
        for (const auto & node: std::vector<XYNode>{n1, n2, n3}) {
            file.write(reinterpret_cast<const char*>(&nodeID), sizeof(nodeID));
            ++nodeID;
            auto data = XYNodeToBinary{}(node);
            uint64_t dataSize = data.size();
            file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
            file.write(reinterpret_cast<const char*>(data.data()), dataSize);
        }
        // Write edges
        file.write(reinterpret_cast<const char*>(&EDGE_COUNT), sizeof(EDGE_COUNT));
        std::vector<std::pair<uint64_t, uint64_t>> edges = {
            {0, 1},
            {1, 2},
            {2, 0}
        };
        for (const auto &[fromID, toID] : edges) {
            file.write(reinterpret_cast<const char*>(&fromID), sizeof(fromID));
            file.write(reinterpret_cast<const char*>(&toID), sizeof(toID));
        }

        file.close();    
    }
    void TearDown() override {
        std::filesystem::remove(filePath);
    }
    fishnet::util::AutomaticTemporaryDirectory temporaryDirectory{};
    std::filesystem::path filePath;
    XYNode n1 {1.0, 2.0};
    XYNode n2 {3.0, 4.0};
    XYNode n3 {5.0, -6.0};
};

TEST_F(BinaryFileAdjacencyTest, init){
    // Non-Existing / Empty File -> No Crash, Empty Graph
    std::filesystem::path nonExistingFile = temporaryDirectory / std::filesystem::path("non_existing_file.bin");
    EXPECT_NO_FATAL_FAILURE( auto binaryAdjacency = BinaryFileAdjacency(AdjacencyMap<XYNode>(), nonExistingFile, XYNodeToBinary{}, XYNodeFromBinary{}); );
    auto nonExistingFileAdjacency = BinaryFileAdjacency(AdjacencyMap<XYNode>(), nonExistingFile, XYNodeToBinary{}, XYNodeFromBinary{});
    EXPECT_EMPTY(nonExistingFileAdjacency.nodes());
    EXPECT_EMPTY(nonExistingFileAdjacency.getAdjacencyPairs());

    // Existing, empty file -> No Crash, Empty Graph
    std::filesystem::path emptyFile = temporaryDirectory / std::filesystem::path("empty_file.bin");
    {
        std::ofstream file(emptyFile);
        file.close();       
    }
    EXPECT_NO_FATAL_FAILURE( auto binaryAdjacency2 = BinaryFileAdjacency(AdjacencyMap<XYNode>(), emptyFile, XYNodeToBinary{}, XYNodeFromBinary{}); );
    auto emptyFileAdjacency = BinaryFileAdjacency(AdjacencyMap<XYNode>(), emptyFile, XYNodeToBinary{}, XYNodeFromBinary{});
    EXPECT_EMPTY(emptyFileAdjacency.nodes());
    EXPECT_EMPTY(emptyFileAdjacency.getAdjacencyPairs());

    // Existing, invalid file -> Crash
    std::filesystem::path invalidFile = temporaryDirectory / std::filesystem::path("invalid_file.bin");
    {
        std::ofstream file(invalidFile);
        file << "Invalid binary data";
        file.close();   
    }
    EXPECT_ANY_THROW( auto binaryAdjacency3 = BinaryFileAdjacency(AdjacencyMap<XYNode>(), invalidFile, XYNodeToBinary{}, XYNodeFromBinary{}););

    // Existing, valid but empty -> No Crash, Empty Graph
    std::filesystem::path validEmptyFile = temporaryDirectory / std::filesystem::path("valid_empty_file.bin");
    {
        std::ofstream file(validEmptyFile, std::ios::binary);
        u_int64_t ZERO = 0;
        file.write(reinterpret_cast<const char*>(&ZERO), sizeof(ZERO)); // zero nodes
        file.write(reinterpret_cast<const char*>(&ZERO), sizeof(ZERO)); // zero edges
        file.close();   
    }
    EXPECT_NO_FATAL_FAILURE( auto binaryAdjacency5 = BinaryFileAdjacency(AdjacencyMap<XYNode>(), validEmptyFile, XYNodeToBinary{}, XYNodeFromBinary{}); );
    auto validEmptyFileAdjacency = BinaryFileAdjacency(AdjacencyMap<XYNode>(), validEmptyFile, XYNodeToBinary{}, XYNodeFromBinary{});
    EXPECT_EMPTY(validEmptyFileAdjacency.nodes());
    EXPECT_EMPTY(validEmptyFileAdjacency.getAdjacencyPairs());
}

TEST_F(BinaryFileAdjacencyTest, load){
    auto binaryAdjacency = BinaryFileAdjacency(AdjacencyMap<XYNode>(), filePath, XYNodeToBinary{}, XYNodeFromBinary{});
    EXPECT_UNSORTED_RANGE_EQ(binaryAdjacency.nodes(), std::vector<XYNode>{n1,n2,n3});
    EXPECT_UNSORTED_RANGE_EQ(fishnet::util::toVector(binaryAdjacency.getAdjacencyPairs()), std::vector<std::pair<XYNode,XYNode>>{{n1,n2},{n2,n3},{n3,n1}});
}

TEST_F(BinaryFileAdjacencyTest, storeAndLoad) {
    AdjacencyMap<XYNode> adjacencyMap;
    adjacencyMap.addNode(n1);
    adjacencyMap.addNode(n2);
    adjacencyMap.addAdjacency(n2,n1);
    auto path = temporaryDirectory / std::filesystem::path("test_store.bin");
    {
        auto adj = BinaryFileAdjacency(std::move(adjacencyMap), path, XYNodeToBinary{}, XYNodeFromBinary{}); 
        // Deconstructor should save the file
    }
    auto loadedAdj = BinaryFileAdjacency(AdjacencyMap<XYNode>(), path, XYNodeToBinary{}, XYNodeFromBinary{});
    EXPECT_UNSORTED_RANGE_EQ(loadedAdj.nodes(), std::vector<XYNode>{n1,n2});
    EXPECT_UNSORTED_RANGE_EQ(fishnet::util::toVector(loadedAdj.getAdjacencyPairs()), std::vector<std::pair<XYNode,XYNode>>{{n2,n1}});
}