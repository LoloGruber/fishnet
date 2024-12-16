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
        connection.executeAndDiscard(CipherQuery::DELETE_ALL());
    }

    u_int16_t port = 7687;
    std::string hostname = "localhost";
    MemgraphConnection connection;
};

TEST_F(MemgraphQueryTest, init){
    EXPECT_NO_FATAL_FAILURE(auto q = CipherQuery());
}

TEST_F(MemgraphQueryTest, mergeAndMatchNode){
    EXPECT_TRUE(connection.executeAndDiscard(CipherQuery().merge(Node{.label=Label::Settlement,.attributes="id:1,value:\"Test\""})));
    EXPECT_TRUE(connection.executeAndDiscard(CipherQuery().merge(Node{.label=Label::Settlement,.attributes="id:2"})));
    EXPECT_TRUE(connection.execute(CipherQuery().match(Node{.name="n",.label=Label::Settlement,.attributes="id:1,value:\"Test\""}).ret("n")));
    auto resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,1); //only one node expected with id 1
}

TEST_F(MemgraphQueryTest, mergeAndMatchEdge){
    CipherQuery i = CipherQuery().merge(Node{.name="f",.label=Label::Settlement,.attributes="id:1"}).endl();
    i.merge(Node{.name="t",.label=Label::Settlement,.attributes="id:2"}).endl();
    i.merge(Relation{.from=Var("f"),.label=Label::neighbours,.to=Var("t")});
    EXPECT_TRUE(connection.executeAndDiscard(i));
    CipherQuery q = CipherQuery().match(Relation{.from=Node{.name="f",.label=Label::Settlement},.to=Node{.name="t",.label=Label::Settlement}}).ret("f","t");
    EXPECT_TRUE(connection.execute(q));
    auto resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,1);
}

TEST_F(MemgraphQueryTest, parameterizedQuery){
    size_t nodeID = 5;
    CipherQuery i = CipherQuery().merge(Node{.label=Label::Settlement,.attributes="id:$nodeID"}).setInt("nodeID",nodeID);
    i.merge(Node{.name="n",.label=Label::Settlement,.attributes="id:-1"});
    EXPECT_TRUE(connection.executeAndDiscard(i));
    CipherQuery getById = CipherQuery().match(Node{"n",Label::Settlement,"id:$nodeID"}).setInt("nodeID",nodeID).ret("n");
    EXPECT_TRUE(connection.execute(getById));
    auto resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,1);
    CipherQuery getAll = CipherQuery().match(Node{"n",Label::Settlement}).ret("n");
    EXPECT_TRUE(connection.execute(getAll));
    resultSize = connection->FetchAll().transform([](auto && v){return v.size();}).value_or(0);
    EXPECT_EQ(resultSize,2);
}