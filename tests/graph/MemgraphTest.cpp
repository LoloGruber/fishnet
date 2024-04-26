#include <gtest/gtest.h>
#include "Testutil.h"
#include <fishnet/MemgraphClient.hpp>
using namespace testutil;
using namespace fishnet::graph;

struct ExampleNode{
    size_t id;
    size_t key() const {
        return id;
    }
};
// static_assert(AdjacencyContainer<MemgraphClient<ExampleNode>,ExampleNode>);


TEST(MemgraphTest, init) {
    u_int16_t port = 7687;
    std::string hostname = "localhost";
    auto mgclient = fishnet::graph::MemgraphClient<ExampleNode>::create(hostname,port);
}