#include <gtest/gtest.h>
#include "CipherQuery.hpp"
#include "MemgraphModel.hpp"
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
    EXPECT_TRUE(CipherQuery().merge(Node{.label=Label::Settlement,.attributes="id:1,value:\"Test\""}).executeAndDiscard(connection));
    EXPECT_TRUE(CipherQuery().merge(Node{.label=Label::Settlement,.attributes="id:2"}).executeAndDiscard(connection));
    EXPECT_TRUE(CipherQuery().match(Node{.name="n",.label=Label::Settlement,.attributes="id:1,value:\"Test\""}).ret("n").execute(connection));
    auto resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,1); //only one node expected with id 1
}

TEST_F(MemgraphQueryTest, mergeAndMatchEdge){
    CipherQuery i = CipherQuery().merge(Node{.name="f",.label=Label::Settlement,.attributes="id:1"}).endl();
    i.merge(Node{.name="t",.label=Label::Settlement,.attributes="id:2"}).endl();
    i.merge(Relation{.from={.name="f"},.label=Label::neighbours,.to={.name="t"}});
    EXPECT_TRUE(i.executeAndDiscard(connection));
    CipherQuery q = CipherQuery().match(Relation{.from={.name="f",.label=Label::Settlement},.to={.name="t",.label=Label::Settlement}}).ret("f","t");
    EXPECT_TRUE(q.execute(connection));
    auto resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,1);
}

TEST_F(MemgraphQueryTest, parameterizedQuery){
    size_t nodeID = 5;
    CipherQuery i = CipherQuery().merge(Node{.label=Label::Settlement,.attributes="id:$nodeID"}).setInt("nodeID",nodeID);
    i.merge(Node{.name="n",.label=Label::Settlement,.attributes="id:-1"});
    EXPECT_TRUE(i.executeAndDiscard(connection));
    CipherQuery getById = CipherQuery().match(Node{"n",Label::Settlement,"id:$nodeID"}).setInt("nodeID",nodeID).ret("n");
    EXPECT_TRUE(getById.execute(connection));
    auto resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,1);
    CipherQuery getAll = CipherQuery().match(Node{"n",Label::Settlement}).ret("n");
    EXPECT_TRUE(getAll.execute(connection));
    resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,2);
}