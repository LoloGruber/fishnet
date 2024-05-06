#include <gtest/gtest.h>
#include <fishnet/AlternativeKeyMap.hpp>
#include "Testutil.h"
using namespace testutil;
using namespace fishnet::util;

class AlternativeKeyMapTest: public ::testing::Test {
protected:
    void SetUp(){
        signIndependentMap = AlternativeKeyMap<size_t,int,std::string>();
        signIndependentMap.insert(1,-1,"One");
        signIndependentMap.insert(2,-2,"Two");
        signIndependentMap.insert(7,-7,"Seven");
        noConversions = AlternativeKeyMap<int,std::string,double>();
        noConversions.insert(1,"One",1.0);
    }

    AlternativeKeyMap<size_t,int,std::string> signIndependentMap;
    AlternativeKeyMap<int,std::string,double> noConversions;
};

TEST_F(AlternativeKeyMapTest, init){
    AlternativeKeyMap<size_t,int,double> intMap;
    intMap.insert(1,-1,1.0);
    intMap.insert(2,-2,2.0);
    EXPECT_SIZE(intMap,2);
    // AlternativeKeyMap<size_t,size_t,std::string> notAllowed;
}

TEST_F(AlternativeKeyMapTest, get) {
    EXPECT_EQ(signIndependentMap.getFromKey(1UL),std::optional<std::string>("One"));
    EXPECT_EQ(signIndependentMap.getFromAlternative(-1),std::optional<std::string>("One"));
    EXPECT_EQ(signIndependentMap.getFromKey(3),std::nullopt);
    EXPECT_EQ(signIndependentMap.getFromKey(7),std::optional<std::string>("Seven"));
    EXPECT_EQ(signIndependentMap.getFromAlternative(7),std::nullopt);
    // auto notAllowedSinceImplicitlyConverted = signIndependentMap.get(1);
    EXPECT_EQ(noConversions.get(1),1.0);
    EXPECT_EQ(noConversions.get("One"),1.0);
}


TEST_F(AlternativeKeyMapTest, getOrElse){
    EXPECT_EQ(signIndependentMap.getFromKeyOrElse(1UL,""),std::string("One"));
    EXPECT_EQ(signIndependentMap.getFromKeyOrElse(3UL,""),std::string(""));
    EXPECT_EQ(signIndependentMap.getFromAlternativeOrElse(-3,""),std::string(""));
    EXPECT_EQ(signIndependentMap.getFromAlternativeOrElse(-1,""),std::string("One"));
    // auto notAllowedSinceImplicitlyConverted = signIndependentMap.getOrElse(1,"");
    EXPECT_EQ(noConversions.getOrElse("Ten",0.0),0.0);
    EXPECT_EQ(noConversions.getOrElse(1,0.0),1.0);
}

TEST_F(AlternativeKeyMapTest, getAssociatedKey) {
    EXPECT_EQ(signIndependentMap.getKey(-1),1UL);
    EXPECT_EQ(signIndependentMap.getAlternative(1UL),-1);
}

TEST_F(AlternativeKeyMapTest, insert) {
    signIndependentMap.insert(4,-4,"Four");
    EXPECT_SIZE(signIndependentMap,4);
    EXPECT_EQ(signIndependentMap.getFromKey(4),std::optional<std::string>("Four"));
    EXPECT_EQ(signIndependentMap.getFromAlternative(-4),std::optional<std::string>("Four"));
    EXPECT_EQ(signIndependentMap.getFromKeyOrElse(4,""),"Four");
    EXPECT_FALSE(signIndependentMap.insert(4,-4,"Four"));
    EXPECT_SIZE(signIndependentMap,4);
    EXPECT_FALSE(signIndependentMap.insert(4,-444,"Four")); //not allowed since primary is not changed
    EXPECT_SIZE(signIndependentMap,4);
}

TEST_F(AlternativeKeyMapTest, updateSecondary) {
    EXPECT_VALUE(signIndependentMap.getFromAlternative(-1));
    EXPECT_TRUE(signIndependentMap.updateAlternative(1,100));
    EXPECT_EMPTY(signIndependentMap.getFromAlternative(-1));
    EXPECT_VALUE(signIndependentMap.getFromAlternative(100));
    EXPECT_FALSE(signIndependentMap.updateAlternative(6,6));
    EXPECT_FALSE(signIndependentMap.updateAlternative(7,-2)); //already a mapping
}

TEST_F(AlternativeKeyMapTest, erase) {
    EXPECT_SIZE(signIndependentMap,3);
    EXPECT_VALUE(signIndependentMap.getFromKey(1));
    EXPECT_VALUE(signIndependentMap.getFromAlternative(-1));
    EXPECT_TRUE(signIndependentMap.eraseFromKey(1));
    EXPECT_SIZE(signIndependentMap,2);
    EXPECT_EMPTY(signIndependentMap.getFromKey(1));
    EXPECT_EMPTY(signIndependentMap.getFromAlternative(-1));
    EXPECT_FALSE(signIndependentMap.eraseFromKey(0));
    EXPECT_SIZE(signIndependentMap,2);
    EXPECT_TRUE(signIndependentMap.eraseFromAlternative(-7));
    EXPECT_SIZE(signIndependentMap,1);
    EXPECT_EMPTY(signIndependentMap.getFromAlternative(-7));
    EXPECT_EMPTY(signIndependentMap.getFromKey(7));
    EXPECT_VALUE(signIndependentMap.getFromKey(2));
    // auto errorSinceConversion = signIndependentMap.erase(2);
    EXPECT_TRUE(noConversions.erase("One"));
    EXPECT_EMPTY(noConversions);
    EXPECT_EQ(noConversions.getOrElse(1,0.0),0.0);
}

TEST_F(AlternativeKeyMapTest, size) {
    EXPECT_EQ(signIndependentMap.size(),3);
    EXPECT_EQ(noConversions.size(),1);
    signIndependentMap.eraseFromKey(1);
    EXPECT_EQ(signIndependentMap.size(),2);
}

TEST_F(AlternativeKeyMapTest, clear) {
    EXPECT_SIZE(signIndependentMap,3);
    signIndependentMap.clear();
    EXPECT_EMPTY(signIndependentMap);
}

TEST_F(AlternativeKeyMapTest, iterators) {
    std::vector<std::string> expected {{
        "One","Two","Seven"
    }};
    std::vector<size_t> expectedKeys {{1,2,7}};

    std::vector<std::string> values;
    std::vector<size_t> keys;
    for(const auto & [primary,value]:signIndependentMap) {
        values.push_back(value);
        keys.push_back(primary);
    }
    EXPECT_UNSORTED_RANGE_EQ(values,expected);
    EXPECT_UNSORTED_RANGE_EQ(keys,expectedKeys);
}