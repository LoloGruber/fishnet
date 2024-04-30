#include <gtest/gtest.h>
#include "Testutil.h"
#include "MemgraphAdjacency.hpp"
using namespace testutil;
using namespace fishnet::graph;

struct ExampleNode{
    size_t id;
    size_t key() const {
        return id;
    }
};
static_assert(AdjacencyContainer<MemgraphAdjacency<ExampleNode>,ExampleNode>);

class MemgraphTest: public ::testing::Test {
protected:
    void SetUp() override {
        if(not mgclient) {
            throw std::runtime_error(mgclient.error());
        }
    }

    u_int16_t port = 7687;
    std::string hostname = "localhost";
    std::expected<MemgraphAdjacency<ExampleNode>,std::string> mgclient = MemgraphAdjacency<ExampleNode>::create(hostname,port);
};

TEST_F(MemgraphTest, create) {
    auto invalidParamsClient = MemgraphAdjacency<ExampleNode>::create("invalidhost",1234);
    EXPECT_EMPTY(invalidParamsClient);
    EXPECT_VALUE(mgclient);
}


TEST_F(MemgraphTest, addAdjacency) {
    ExampleNode n {1};
    ExampleNode n2 {2};
    mgclient->addAdjacency(n,n2);
}

