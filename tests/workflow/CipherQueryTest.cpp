#include <gtest/gtest.h>
#include "CipherQuery.hpp"
#include "Testutil.h"

using namespace testutil;

class MemgraphQueryTest: public ::testing::Test {
protected:
    void SetUp() override {
        auto connectionExp = MemgraphConnection::create(hostname,port);
        if(not connectionExp) {
            throw std::runtime_error(connectionExp.error());
        }
        connection = std::move(connectionExp.value());
    }

    void TearDown() override {
        CipherQuery::DELETE_ALL().executeAndDiscard(connection);
    }

    u_int16_t port = 7687;
    std::string hostname = "localhost";
    MemgraphConnection connection;
};

TEST_F(MemgraphQueryTest, init){
    EXPECT_NO_FATAL_FAILURE(auto q = CipherQuery());
}

TEST_F(MemgraphQueryTest, mergeAndMatchNode){
    EXPECT_TRUE(CipherQuery().merge().node("Node","id:1,value:\"Test\"").executeAndDiscard(connection));
    EXPECT_TRUE(CipherQuery().match().node('n',"Node","id:1,value:\"Test\"").ret('n').execute(connection));
    auto resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,1);
}

TEST_F(MemgraphQueryTest, mergeAndMatchEdge){
    CipherQuery i = CipherQuery().merge().node('f',"Node","id:1").endl();
    i.merge().node('t',"Node","id:2").endl();
    i.merge().node('f').edge("adjacent",CipherQuery::RelationshipDirection::LEFT).node('t');
    EXPECT_TRUE(i.executeAndDiscard(connection));
    CipherQuery q = CipherQuery().match().node('f',"Node").edge("adjacent",CipherQuery::RelationshipDirection::LEFT).node('t',"Node").ret('f','t');
    EXPECT_TRUE(q.execute(connection));
    auto resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,1);
}

TEST_F(MemgraphQueryTest, parameterizedQuery){
    size_t nodeID = 5;
    CipherQuery i = CipherQuery().merge().node("Node","id:$nodeID").setInt("nodeID",nodeID);
    i.merge().node('n',"Node","id:-1");
    EXPECT_TRUE(i.executeAndDiscard(connection));
    CipherQuery getById = CipherQuery().match().node('n',"Node","id:$nodeID").setInt("nodeID",nodeID).ret('n');
    EXPECT_TRUE(getById.execute(connection));
    auto resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,1);
    CipherQuery getAll = CipherQuery().match().node('n',"Node").ret('n');
    EXPECT_TRUE(getAll.execute(connection));
    resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,2);
}