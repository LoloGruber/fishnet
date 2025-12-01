#include <gtest/gtest.h>
#include "Testutil.h"
#include <fishnet/MemgraphClient.hpp>
#include "JobAdjacency.hpp"
#include "../WorkflowTestEnvironment.hpp"

using namespace testutil;
class ConcurrentSessionsTest: public ::testing::Test {
protected:
    static void SetUpTestSuite(){
        connection = MemgraphConnection::create(WorkflowTestEnvironment::memgraphParams()).value_or_throw();
        adj = JobAdjacency(MemgraphConnection(connection));
    }

    void SetUp() override {
        MemgraphConnection::setSession(initialSession);
        adj->addAdjacency(filterJob,neighboursJob);
        assert(adj->nodes().size()==2);
    }

    void TearDown() {
        MemgraphConnection::setSession(initialSession);
        adj->clear();
    }

    static void TearDownTestSuite(){
        connection.executeAndDiscard(CipherQuery::DELETE_ALL());
    }

    static inline MemgraphConnection connection;
    static inline std::optional<JobAdjacency> adj;
    Session initialSession = Session::makeUnique(this->connection);

    Job filterJob {1,"filter.json",JobType::FILTER,JobState::RUNNABLE};
    Job neighboursJob {2,"neighbours.json",JobType::NEIGHBOURS,JobState::RUNNABLE};
};
TEST_F(ConcurrentSessionsTest, concurrentDAG){
    Session other = Session::makeUnique(this->connection);
    EXPECT_NE(initialSession.id(),other.id());
    MemgraphConnection::setSession(other);
    adj->addNode(filterJob);
    EXPECT_SIZE(adj->nodes(),1);
    MemgraphConnection::setSession(initialSession);
    EXPECT_SIZE(adj->nodes(),2);
    MemgraphConnection::setSession(other);
    adj->clear();
    EXPECT_EMPTY(adj->nodes());
}

TEST_F(ConcurrentSessionsTest, getSession){
    Session empty = Session::makeUnique(connection);
    EXPECT_NE(empty.id(),initialSession.id());
    MemgraphConnection::setSession(empty);
    EXPECT_EMPTY(adj->getAdjacencyPairs());
    auto copyOfInitOpt = Session::of(connection,initialSession.id());
    EXPECT_VALUE(copyOfInitOpt);
    auto session = copyOfInitOpt.value();
    MemgraphConnection::setSession(session);
    EXPECT_SIZE(adj->getAdjacencyPairs(),1);
}

TEST_F(ConcurrentSessionsTest, reset){
    MemgraphConnection::resetSession();
    EXPECT_FALSE(MemgraphConnection::hasSession());
}


