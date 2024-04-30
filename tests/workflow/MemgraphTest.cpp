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
    ExampleNode n {12,fileRef};
    ExampleNode n2 {13,fileRef};
    EXPECT_TRUE(mgAdj->addAdjacency(n,n2));
}

TEST_F(MemgraphTest, addNode){
    ExampleNode n {42,fileRef};
    EXPECT_TRUE(mgAdj->addNode(n));
    EXPECT_FALSE(mgAdj->addNode(n));
}

