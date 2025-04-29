#include <gtest/gtest.h>
#include "Testutil.h"
#include <fishnet/MemgraphClient.hpp>
#include "JobAdjacency.hpp"
using namespace testutil;
class ConcurrentSessionsTest: public ::testing::Test {
protected:
    void SetUp() override {
        MemgraphConnection::setSession(initialSession);
        JobAdjacency adj {MemgraphConnection(connection)};
        adj.addAdjacency(filterJob,neighboursJob);
        assert(adj.nodes().size()==2);
    }

    void TearDown() {
        MemgraphConnection::setSession(initialSession);
        JobAdjacency(MemgraphConnection(connection)).clear();
    }
    MemgraphConnection connection = MemgraphConnection::create("localhost",7687).value_or_throw();
    Session initialSession = Session::makeUnique(this->connection);
    Job filterJob {1,"filter.json",JobType::FILTER,JobState::RUNNABLE};
    Job neighboursJob {2,"neighbours.json",JobType::NEIGHBOURS,JobState::RUNNABLE};
};
TEST_F(ConcurrentSessionsTest, concurrentDAG){
    Session other = Session::makeUnique(this->connection);
    EXPECT_NE(initialSession.id(),other.id());
    MemgraphConnection::setSession(other);
    JobAdjacency adj {MemgraphConnection{connection}};
    adj.addNode(filterJob);
    EXPECT_SIZE(adj.nodes(),1);
    MemgraphConnection::setSession(initialSession);
    EXPECT_SIZE(adj.nodes(),2);
    MemgraphConnection::setSession(other);
    adj.clear();
    EXPECT_EMPTY(adj.nodes());
}

TEST_F(ConcurrentSessionsTest, getSession){
    Session empty = Session::makeUnique(connection);
    EXPECT_NE(empty.id(),initialSession.id());
    MemgraphConnection::setSession(empty);
    JobAdjacency adj {MemgraphConnection{connection}};
    EXPECT_EMPTY(adj.getAdjacencyPairs());
    auto copyOfInitOpt = Session::of(connection,initialSession.id());
    EXPECT_VALUE(copyOfInitOpt);
    auto session = copyOfInitOpt.value();
    MemgraphConnection::setSession(session);
    EXPECT_SIZE(adj.getAdjacencyPairs(),1);
}

TEST_F(ConcurrentSessionsTest, reset){
    MemgraphConnection::resetSession();
    EXPECT_FALSE(MemgraphConnection::hasSession());
}


