#include <gtest/gtest.h>
#include "Testutil.h"
#include "MemgraphAdjacency.hpp"
using namespace testutil;
using namespace fishnet::graph;

struct ExampleNode{
    size_t id;
    FileReference fileRef;
    size_t key() const {
        return id;
    }
    const FileReference & file() const {
        return fileRef;
    }
};
static_assert(AdjacencyContainer<MemgraphAdjacency<ExampleNode>,ExampleNode>);

/*
Show all query: 

MATCH (n:Node)-[r:stored]->(f:File)
MATCH (n1:Node)-[a:adj]->(n2:Node)
RETURN n,r,f,n2,n1,a

*/

class MemgraphTest: public ::testing::Test {
protected:
    void SetUp() override {
        if(not mgAdj) {
            throw std::runtime_error(mgAdj.error());
        }
        
        auto optFileRef = mgAdj->getDatabaseConnection().addFileReference("test.shp");
        if(optFileRef){
            fileRef = optFileRef.value();
        }
        else throw std::runtime_error("Coult not create file reference");
    }

    void TearDown() override {
        if(mgAdj)
            mgAdj->getDatabaseConnection().clearAll();
    }

    u_int16_t port = 7687;
    std::string hostname = "localhost";
    std::expected<MemgraphAdjacency<ExampleNode>,std::string> mgAdj = MemgraphAdjacency<ExampleNode>::create(hostname,port);
    FileReference fileRef;
};

TEST_F(MemgraphTest, create) {
    auto invalidParamsClient = MemgraphAdjacency<ExampleNode>::create("invalidhost",1234);
    EXPECT_EMPTY(invalidParamsClient);
    EXPECT_VALUE(mgAdj);
}


TEST_F(MemgraphTest, addAdjacency) {
    ExampleNode n {9999,fileRef};
    ExampleNode n2 {10000,fileRef};
    EXPECT_TRUE(mgAdj->addAdjacency(n,n2));
   EXPECT_TRUE(mgAdj->hasAdjacency(n,n2));
}

TEST_F(MemgraphTest, addAdjacencies){
    const size_t amount = 100;
    std::vector<std::pair<ExampleNode,ExampleNode>> edges;
    for(size_t index = 0;index < amount*2;index += 2){
        edges.emplace_back(ExampleNode(index,fileRef),ExampleNode(index+1,fileRef));
    }
    EXPECT_TRUE(mgAdj->addAdjacencies(edges));
    for(const auto & [from,to]:edges) {
        EXPECT_TRUE(mgAdj->hasAdjacency(from,to));
    }
}

TEST_F(MemgraphTest, addNode){
    ExampleNode n {42,fileRef};
    EXPECT_TRUE(mgAdj->addNode(n));
    EXPECT_FALSE(mgAdj->addNode(n));
    EXPECT_TRUE(mgAdj->contains(n));
}

TEST_F(MemgraphTest, addNodes){
    const u_int32_t amount = 1000;
    FileReference & ref = fileRef;
    auto nodes = std::views::iota(0ul,amount)
        | std::views::transform([&ref](size_t index){return ExampleNode{index,ref};});
    EXPECT_TRUE(mgAdj->addNodes(nodes));
    for(const auto & node: nodes) {
        EXPECT_TRUE(mgAdj->contains(node));
    }
}

TEST_F(MemgraphTest, removeNode) {
    ExampleNode f {2,fileRef};
    ExampleNode t {1,fileRef};
    EXPECT_TRUE(mgAdj->addAdjacency(f,t));
    EXPECT_TRUE(mgAdj->hasAdjacency(f,t));
    EXPECT_TRUE(mgAdj->removeNode(f));
    EXPECT_FALSE(mgAdj->hasAdjacency(f,t));
    EXPECT_TRUE(mgAdj->contains(t));
    EXPECT_FALSE(mgAdj->contains(f));
}

TEST_F(MemgraphTest, removeAdjacency) {
    ExampleNode f {111111,fileRef};
    ExampleNode t {222222,fileRef};
    EXPECT_TRUE(mgAdj->addAdjacency(f,t));
    EXPECT_TRUE(mgAdj->hasAdjacency(f,t));
    EXPECT_TRUE(mgAdj->removeAdjacency(f,t));
    EXPECT_FALSE(mgAdj->hasAdjacency(f,t));
    EXPECT_TRUE(mgAdj->contains(f));
    EXPECT_TRUE(mgAdj->contains(t));
}

TEST_F(MemgraphTest, clear) {
    // EXPECT_TRUE(mgAdj->getDatabaseConnection().clearAll());
}

