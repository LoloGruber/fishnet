#include <gtest/gtest.h>

#include "Testutil.h"
#include "NestedMap.hpp"
using namespace testutil;
using namespace util;

class NestedMapTest: public ::testing::Test {
protected:
    void SetUp() override {
        map = NestedMap<size_t,const char *, double>();
        map.insert(0,"Zero",0.0);
        map.insert(1,"One",1.0);
    }

    NestedMap<size_t,const char *,double> map;
};

TEST_F(NestedMapTest, get) {
    EXPECT_EQ(map.get(0,"Zero").value(),0.0);
    EXPECT_EMPTY(map.get(0,"One"));
    EXPECT_EQ(map.get(1,"One").value(),1.0);
    EXPECT_EMPTY(map.get(1,"ONE"));
}

TEST_F(NestedMapTest, getOrElse) {
    EXPECT_EQ(map.getOrElse(0,"Zero",-1.0),0.0);
    EXPECT_EQ(map.getOrElse(0,"ZERO",-1.0),-1.0);
}

TEST_F(NestedMapTest, erase) {
    EXPECT_EQ(map.size(),2);
    map.erase(123456789,"NotExists");
    EXPECT_EQ(map.size(),2);
    EXPECT_TRUE(map.contains(0,"Zero"));
    map.erase(0,"Zero");
    EXPECT_EQ(map.size(0),0);
    EXPECT_EQ(map.size(),2); //outer key does not get deleted
    EXPECT_FALSE(map.contains(0,"Zero"));
    EXPECT_EMPTY(map.get(0,"Zero"));
}

TEST_F(NestedMapTest, contains) {
    EXPECT_TRUE(map.contains(1,"One"));
    EXPECT_FALSE(map.contains(3,"Three"));
    EXPECT_FALSE(map.contains(1,"one"));
    EXPECT_TRUE(map.contains(0,"Zero"));
    EXPECT_TRUE(map.containsOuterKey(0));
    EXPECT_FALSE(map.containsOuterKey(-1));
}

TEST_F(NestedMapTest, try_insert) {
    EXPECT_FALSE(map.try_insert(0,"Zero",3.14));
    EXPECT_TRUE(map.try_insert(2,"Two",2.0));
    EXPECT_TRUE(map.contains(0,"Zero"));
    EXPECT_EQ(map.get(2,"Two").value(),2.0);
    EXPECT_TRUE(map.try_insert(2,"Zwei",-2.0));
    EXPECT_EQ(map.size(),3);
    EXPECT_EQ(map.size(0),1);
    EXPECT_EQ(map.size(2),2); // 2.0 and -2.0
    EXPECT_EQ(map.get(2,"Zwei").value(),-2.0);
    EXPECT_EQ(map.get(2,"Two").value(),2.0);
}

TEST_F(NestedMapTest, insert) {
    EXPECT_FALSE(map.contains(2,"Two"));
    map.insert(2,"Two",2.0);
    EXPECT_TRUE(map.contains(2,"Two"));
    EXPECT_EQ(map.get(0,"Zero").value(),0.0);
    double newValue = 1000.0;
    map.insert(0,"Zero",newValue);
    EXPECT_EQ(map.get(0,"Zero").value(),newValue);
    EXPECT_EQ(map.size(),3);
}

TEST_F(NestedMapTest, innerMap) {
    auto viewOnInnerKeys  = map.innerMap(0);
    EXPECT_SIZE(viewOnInnerKeys,1);
    for(const auto & [key,value]: viewOnInnerKeys) {
        EXPECT_EQ(key, "Zero");
        EXPECT_EQ(value,0.0);
    }
}

TEST_F(NestedMapTest, eraseInnerKey) {
    EXPECT_EQ(map.size(),2);
    map.eraseInnerKey("One");
    EXPECT_EQ(map.size(),2); // outer key does not get deleted
    EXPECT_EQ(map.size(1),0);
}

