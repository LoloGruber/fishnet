#include <gtest/gtest.h>
#include "Testutil.h"
#include <fishnet/MemgraphAdjacency.hpp>
#include <fishnet/CachingMemgraphAdjacency.hpp>
#include <fishnet/GraphFactory.hpp>
#include "WorkflowTestEnvironment.hpp"

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

    bool operator==(const ExampleNode & other) const noexcept {
        return this->id == other.id;
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
    static void SetUpTestSuite() {
        mgAdj = MemgraphConnection::create(params)
            .transform([](auto && connection){return CachingMemgraphAdjacency<ExampleNode>(MemgraphClient(std::move(connection)));});
        if(not mgAdj) {
            throw std::runtime_error(mgAdj.error());
        }
    }

    void SetUp() override {
        auto optFileRef = mgAdj->getDatabaseConnection().addFileReference("test.shp");
        if(optFileRef){
            fileRef = optFileRef.value();
        }
        else throw std::runtime_error("Could not create file reference");
    }

    void TearDown() override {
        if(mgAdj) {
            mgAdj->clear();
            mgAdj->getDatabaseConnection().clearAll();
        }
    }

    static inline mg::Client::Params params = WorkflowTestEnvironment::memgraphParams();
    static inline std::expected<CachingMemgraphAdjacency<ExampleNode>,std::string> mgAdj = std::unexpected("Not initialized");
    FileReference fileRef;
};

TEST_F(MemgraphTest, create) {
    auto invalidParamsClient = MemgraphConnection::create({.host="invalidhost", .port=1234});
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

TEST_F(MemgraphTest, removeNodes) {
    const u_int32_t amount = 250;
    FileReference & ref = fileRef;
    auto nodes = std::views::iota(0ul,amount)
        | std::views::transform([&ref](size_t index){return ExampleNode{index,ref};});
    EXPECT_TRUE(mgAdj->addNodes(nodes));
    for(const auto & node: nodes) {
        EXPECT_TRUE(mgAdj->contains(node));
    }
    auto indexUtilDelete = amount/2;
    auto toBeRemoved = nodes | std::views::take(indexUtilDelete);
    std::vector<ExampleNode> notContained = {
        {123456789,ref},{987654321,ref}
    };
    EXPECT_TRUE(mgAdj->removeNodes(toBeRemoved));
    EXPECT_TRUE(mgAdj->removeNodes(notContained));
    for(const auto & node: nodes) {
        if(node.id >= indexUtilDelete){
            EXPECT_TRUE(mgAdj->contains(node));
        }else {
            EXPECT_FALSE(mgAdj->contains(node));
        }
    }
    for(const auto & node: notContained) {
        EXPECT_FALSE(mgAdj->contains(node));
    }


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

TEST_F(MemgraphTest, removeAdjacencies) {
    const size_t amount = 100;
    std::vector<std::pair<ExampleNode,ExampleNode>> edges;
    for(size_t index = 0;index < amount*2;index += 2){
        edges.emplace_back(ExampleNode(index,fileRef),ExampleNode(index+1,fileRef));
    }
    EXPECT_TRUE(mgAdj->addAdjacencies(edges));
    for(const auto & [from,to]:edges) {
        EXPECT_TRUE(mgAdj->hasAdjacency(from,to));
    }
    auto removedEdges = std::views::all(edges) | std::views::filter([](const auto & pair){
        return pair.first.key() != 0;
    });
    EXPECT_SIZE(removedEdges,amount-1);
    EXPECT_TRUE(mgAdj->removeAdjacencies(removedEdges));
    for(const auto & [from,to]:removedEdges) {
        EXPECT_FALSE(mgAdj->hasAdjacency(from,to));
    }
    const auto & [from,to] = edges.front(); //first edge was not removed
    EXPECT_TRUE(mgAdj->hasAdjacency(from,to));
}

TEST_F(MemgraphTest, contains) {
    ExampleNode added {1,fileRef};
    ExampleNode notAdded {2,fileRef};
    EXPECT_TRUE(mgAdj->addNode(added));
    EXPECT_TRUE(mgAdj->contains(added));
    EXPECT_FALSE(mgAdj->contains(notAdded));
    EXPECT_TRUE(mgAdj->contains(ExampleNode(1,fileRef)));
    EXPECT_FALSE(mgAdj->contains(ExampleNode(404,fileRef)));
}

TEST_F(MemgraphTest, hasAdjacency) {
    ExampleNode from {1,fileRef};
    ExampleNode to {2,fileRef};
    EXPECT_TRUE(mgAdj->addAdjacency(from,to));
    EXPECT_TRUE(mgAdj->hasAdjacency(from,to));
    EXPECT_FALSE(mgAdj->hasAdjacency(to,from));
    EXPECT_TRUE(mgAdj->contains(from));
    EXPECT_TRUE(mgAdj->contains(to));
    EXPECT_TRUE(mgAdj->removeAdjacency(from,to));
    EXPECT_FALSE(mgAdj->hasAdjacency(from,to));
    EXPECT_FALSE(mgAdj->hasAdjacency(to,from));
}

TEST_F(MemgraphTest, adjacency) {
    ExampleNode n1 {1,fileRef};
    ExampleNode n2 {2,fileRef};
    ExampleNode n3 {3,fileRef};
    ExampleNode n4 {4,fileRef};
    mgAdj->addAdjacency(n1,n2);
    mgAdj->addAdjacency(n1,n3);
    mgAdj->addAdjacency(n3,n4);
    EXPECT_EMPTY(mgAdj->adjacency(n2));
    EXPECT_SIZE(mgAdj->adjacency(n3),1);
    EXPECT_CONTAINS(mgAdj->adjacency(n3),n4);
    EXPECT_EMPTY(mgAdj->adjacency(n2));
    EXPECT_SIZE(mgAdj->adjacency(n1),2);
    auto neighboursOfN1 = mgAdj->adjacency(n1);
    EXPECT_CONTAINS(neighboursOfN1,n2);
    EXPECT_CONTAINS(neighboursOfN1,n3);
}

TEST_F(MemgraphTest, nodes) {
    ExampleNode n1 {1,fileRef};
    ExampleNode n2 {2,fileRef};
    mgAdj->addNode(n1);
    EXPECT_CONTAINS(mgAdj->nodes(),n1);
    EXPECT_NOT_CONTAINS(mgAdj->nodes(),n2);
    mgAdj->addNode(n2);
    EXPECT_SIZE(mgAdj->nodes(),2);
    EXPECT_CONTAINS_ALL(mgAdj->nodes(),std::vector<ExampleNode>{{n1,n2}});
    mgAdj->addAdjacency(ExampleNode(3,fileRef),ExampleNode(4,fileRef));
    EXPECT_SIZE(mgAdj->nodes(),4);
    EXPECT_CONTAINS(mgAdj->nodes(), ExampleNode(3,fileRef));
    mgAdj->addAdjacency(n1,n2);
    mgAdj->removeAdjacency(n1,n2);
    EXPECT_SIZE(mgAdj->nodes(),4);
}

TEST_F(MemgraphTest, adjacencyPairs) {
    ExampleNode n1 {1,fileRef};
    ExampleNode n2 {2,fileRef};
    ExampleNode n3 {3,fileRef};
    ExampleNode n4 {4,fileRef};
    mgAdj->addAdjacency(n1,n2);
    mgAdj->addAdjacency(n1,n3);
    mgAdj->addAdjacency(n3,n4);
    auto edges = mgAdj->getAdjacencyPairs();
    EXPECT_SIZE(edges,3);
    std::vector<std::pair<ExampleNode,ExampleNode>> expected = {
        {n1,n2},{n1,n3},{n3,n4}
    };
    EXPECT_CONTAINS_ALL(edges,expected);
}

TEST_F(MemgraphTest, clear) {
    ExampleNode n1 {1,fileRef};
    ExampleNode n2 {2,fileRef};
    ExampleNode n3 {3,fileRef};
    ExampleNode n4 {4,fileRef};
    mgAdj->addAdjacency(n1,n2);
    mgAdj->addAdjacency(n1,n3);
    mgAdj->addAdjacency(n3,n4);
    EXPECT_SIZE(mgAdj->nodes(),4);
    mgAdj->clear();
    EXPECT_EMPTY(mgAdj->nodes());
    EXPECT_EMPTY(mgAdj->getAdjacencyPairs());
}

TEST_F(MemgraphTest, undirectedGraph){
    std::expected<MemgraphAdjacency<ExampleNode>,std::string> mgAdj = MemgraphConnection::create(params).transform([](auto && c){return MemgraphAdjacency<ExampleNode>(MemgraphClient(std::move(c)));});
    auto g = fishnet::graph::GraphFactory::UndirectedGraph<ExampleNode>(std::move(mgAdj.value()));
    ExampleNode n1 {1,fileRef};
    ExampleNode n2 {2,fileRef};
    g.addEdge(n1,n2);
    auto s = fishnet::util::size(g.getNeighbours(n1));
    EXPECT_EQ(s,1);

}
