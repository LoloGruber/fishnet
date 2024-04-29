#include <gtest/gtest.h>
#include "Testutil.h"
#include <fishnet/MemgraphClient.hpp>
using namespace testutil;
using namespace fishnet::graph;

struct ExampleNode{
    int64_t id;
    int64_t key() const {
        return id;
    }
};
static_assert(AdjacencyContainer<MemgraphClient<ExampleNode>,ExampleNode>);

class MemgraphTest: public ::testing::Test {
protected:
    void SetUp() override {
        if(not mgclient) {
            throw std::runtime_error("Could not connected to Database\nAborting Tests");
        }
    }

    u_int16_t port = 7687;
    std::string hostname = "localhost";
    std::optional<MemgraphClient<ExampleNode>> mgclient = MemgraphClient<ExampleNode>::create(hostname,port);
};


TEST_F(MemgraphTest, addAdjacency) {
    ExampleNode n {1};
    ExampleNode n2 {2};
    mgclient->addAdjacency(n,n2);
}

