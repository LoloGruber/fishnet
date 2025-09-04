#include <gtest/gtest.h>
#include <fishnet/JSONFileAdjacency.hpp>
#include <fishnet/GraphFactory.hpp>
#include <fishnet/AdjacencyMap.hpp>
#include <fishnet/TemporaryDirectiory.h>
#include "XYNode.h"
#include "Testutil.h"

using namespace fishnet::graph;
using namespace nlohmann;
using namespace testutil;

struct XYNodeJSON{
    static json operator()(const XYNode & node){
        return json{{"x",node.getX()},{"y",node.getY()}};
    }
    static XYNode operator()(const json & j){
        return XYNode{j.at("x").get<double>(),j.at("y").get<double>()};
    }
};

class JSONAdjacencyTest : public ::testing::Test {
protected:
    void SetUp() override { 
        filePath = temporaryDirectory / std::filesystem::path("test_graph.json");
        json j;
        j["nodes"] = json::array();
        j["edges"] = json::array();
        j["nodes"].push_back({{"id",1},{"data",XYNodeJSON{}(n1)}});
        j["nodes"].push_back({{"id",2},{"data",XYNodeJSON{}(n2)}});
        j["nodes"].push_back({{"id",3},{"data",XYNodeJSON{}(n3)}});
        j["edges"].push_back({{"from",1},{"to",2}});
        j["edges"].push_back({{"from",2},{"to",3}});
        j["edges"].push_back({{"from",3},{"to",1}});
        std::ofstream file(filePath);
        file << j.dump(4);
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

TEST_F(JSONAdjacencyTest, init){
    // Non-Existing / Empty File -> No Crash, Empty Graph
    std::filesystem::path nonExistingFile = temporaryDirectory / std::filesystem::path("non_existing_file.json");
    EXPECT_NO_FATAL_FAILURE( auto jsonAdjacency = JSONFileAdjacency(AdjacencyMap<XYNode>(), nonExistingFile, XYNodeJSON{}, XYNodeJSON{}); );
    auto nonExistingFileAdjacency = JSONFileAdjacency(AdjacencyMap<XYNode>(), nonExistingFile, XYNodeJSON{}, XYNodeJSON{});
    EXPECT_EMPTY(nonExistingFileAdjacency.nodes());
    EXPECT_EMPTY(nonExistingFileAdjacency.getAdjacencyPairs());

    // Existing, empty file -> No Crash, Empty Graph
    std::filesystem::path emptyFile = temporaryDirectory / std::filesystem::path("empty_file.json");
    {
        std::ofstream file(emptyFile);
        file.close();       
    }
    EXPECT_NO_FATAL_FAILURE( auto jsonAdjacency2 = JSONFileAdjacency(AdjacencyMap<XYNode>(), emptyFile, XYNodeJSON{}, XYNodeJSON{}); );
    auto emptyFileAdjacency = JSONFileAdjacency(AdjacencyMap<XYNode>(), emptyFile, XYNodeJSON{}, XYNodeJSON{});
    EXPECT_EMPTY(emptyFileAdjacency.nodes());
    EXPECT_EMPTY(emptyFileAdjacency.getAdjacencyPairs());

    // Existing, invalid file -> Crash
    std::filesystem::path invalidFile = temporaryDirectory / std::filesystem::path("invalid_file.json");
    {
        std::ofstream file(invalidFile);
        file << "This is not a valid JSON file";
        file.close();   
    }
    EXPECT_ANY_THROW( auto jsonAdjacency3 = JSONFileAdjacency(AdjacencyMap<XYNode>(), invalidFile, XYNodeJSON{}, XYNodeJSON{}););

    // Existing, invalid file -> Missing edges key -> Crash
    std::filesystem::path missingEdgesKeyFile = temporaryDirectory / std::filesystem::path("missing_edges_key_file.json");
    {
        std::ofstream file(missingEdgesKeyFile);
        file << R"({"nodes": [{"id": 1, "data": {"x": 1.0, "y": 2.0}}]})";
        file.close();   
    }
    EXPECT_ANY_THROW( auto jsonAdjacency4 = JSONFileAdjacency(AdjacencyMap<XYNode>(), missingEdgesKeyFile, XYNodeJSON{}, XYNodeJSON{}););

    // Existing, valid but empty -> No Crash, Empty Graph
    std::filesystem::path validEmptyFile = temporaryDirectory / std::filesystem::path("valid_empty_file.json");
    {
        std::ofstream file(validEmptyFile);
        file << R"({"nodes": [], "edges": []})";
        file.close();   
    }
    EXPECT_NO_FATAL_FAILURE( auto jsonAdjacency5 = JSONFileAdjacency(AdjacencyMap<XYNode>(), validEmptyFile, XYNodeJSON{}, XYNodeJSON{}); );
    auto validEmptyFileAdjacency = JSONFileAdjacency(AdjacencyMap<XYNode>(), validEmptyFile, XYNodeJSON{}, XYNodeJSON{});
    EXPECT_EMPTY(validEmptyFileAdjacency.nodes());
    EXPECT_EMPTY(validEmptyFileAdjacency.getAdjacencyPairs());
}

TEST_F(JSONAdjacencyTest, load){
    auto jsonAdjacency = JSONFileAdjacency(AdjacencyMap<XYNode>(), filePath, XYNodeJSON{}, XYNodeJSON{});
    EXPECT_UNSORTED_RANGE_EQ(jsonAdjacency.nodes(), std::vector<XYNode>{n1,n2,n3});
    EXPECT_UNSORTED_RANGE_EQ(fishnet::util::toVector(jsonAdjacency.getAdjacencyPairs()), std::vector<std::pair<XYNode,XYNode>>{{n1,n2},{n2,n3},{n3,n1}});
}

TEST_F(JSONAdjacencyTest, storeAndLoad) {
    AdjacencyMap<XYNode> adjacencyMap;
    adjacencyMap.addNode(n1);
    adjacencyMap.addNode(n2);
    adjacencyMap.addAdjacency(n2,n1);
    auto path = temporaryDirectory / std::filesystem::path("test_store.json");
    {
        auto adj = JSONFileAdjacency(std::move(adjacencyMap), path, XYNodeJSON{}, XYNodeJSON{}); 
        // Deconstructor should save the file
    }
    auto loadedAdj = JSONFileAdjacency(AdjacencyMap<XYNode>(), path, XYNodeJSON{}, XYNodeJSON{});
    EXPECT_UNSORTED_RANGE_EQ(loadedAdj.nodes(), std::vector<XYNode>{n1,n2});
    EXPECT_UNSORTED_RANGE_EQ(fishnet::util::toVector(loadedAdj.getAdjacencyPairs()), std::vector<std::pair<XYNode,XYNode>>{{n2,n1}});
}