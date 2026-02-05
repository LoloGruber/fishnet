#include <gtest/gtest.h>
#include <fishnet/TestUtil.hpp>
#include <fishnet/BidirectionalMap.hpp>

using namespace testutil;

class BidirectionalHashMapTest: public ::testing::Test {
protected:
    void SetUp() override {
        biHashMap = fishnet::util::BidirectionalHashMap<size_t,std::string>();
        biHashMap.insert(1,"One");
        biHashMap.insert(2,"Two");
        biHashMap.insert(3,"Three");
    }  
    fishnet::util::BidirectionalHashMap<size_t,std::string> biHashMap;
};

TEST_F(BidirectionalHashMapTest, initList) {
    std::initializer_list<std::pair<int,double>> initList {{1,1.0},{2,2.0}};
    auto biMap = fishnet::util::BidirectionalHashMap<int,double>(initList);
    EXPECT_SIZE(biMap,2);
}

TEST_F(BidirectionalHashMapTest, iterate){
    for(auto & [key,value]:biHashMap){
        value = "Modified";
    }
    EXPECT_EQ(biHashMap.getTo(1).value(),"Modified");
}

TEST_F(BidirectionalHashMapTest, constIterate){
    for(const auto & [key,value]:biHashMap){
        // value = "Modified"; // Should cause a compile error if uncommented
        EXPECT_EQ(biHashMap.getTo(key).value(),value);
    }
}

TEST_F(BidirectionalHashMapTest, inverseIterate){
    for(auto it = biHashMap.inverseBegin(); it != biHashMap.inverseEnd(); ++it){
        it->second = 42;
    }
    EXPECT_EQ(biHashMap.getFrom("One").value(),42);
}

TEST_F(BidirectionalHashMapTest, inverseConstIterate){
    for(auto it = biHashMap.cInverseBegin(); it != biHashMap.cInverseEnd(); ++it){
        //it->second = 42; // Should cause a compile error if uncommented
        EXPECT_EQ(biHashMap.getFrom(it->first).value(),it->second);
    }
}

TEST_F(BidirectionalHashMapTest, empty){
    EXPECT_FALSE(biHashMap.empty());
    biHashMap.eraseFrom(1);
    biHashMap.eraseFrom(2);
    biHashMap.eraseFrom(3);
    EXPECT_TRUE(biHashMap.empty());
    auto emptyMap = fishnet::util::BidirectionalHashMap<int,int>();
    EXPECT_TRUE(emptyMap.empty());
}

TEST_F(BidirectionalHashMapTest, size){
    EXPECT_EQ(biHashMap.size(),3);
    biHashMap.eraseFrom(1);
    EXPECT_EQ(biHashMap.size(),2);
    biHashMap.eraseTo("Two");
    EXPECT_EQ(biHashMap.size(),1);
    auto emptyMap = fishnet::util::BidirectionalHashMap<int,int>();
    EXPECT_EQ(emptyMap.size(),0);
}

TEST_F(BidirectionalHashMapTest, contains){
    EXPECT_TRUE(biHashMap.containsFrom(1));
    EXPECT_TRUE(biHashMap.containsTo("Two"));
    EXPECT_TRUE(biHashMap.contains(3));
    EXPECT_TRUE(biHashMap.contains("Three"));
    EXPECT_FALSE(biHashMap.containsFrom(42));
    EXPECT_FALSE(biHashMap.containsTo("NonExisting"));
    EXPECT_FALSE(biHashMap.contains(42));
}

TEST_F(BidirectionalHashMapTest, eraseFrom){
    EXPECT_EQ(biHashMap.size(),3);
    auto erased = biHashMap.eraseFrom(1);
    EXPECT_TRUE(erased);
    EXPECT_EQ(biHashMap.size(),2);
    EXPECT_FALSE(biHashMap.containsFrom(1));
    EXPECT_FALSE(biHashMap.containsTo("One"));
    auto notErased = biHashMap.eraseFrom(42);
    EXPECT_FALSE(notErased);
    EXPECT_EQ(biHashMap.size(),2);
}

TEST_F(BidirectionalHashMapTest, eraseTo){
    EXPECT_EQ(biHashMap.size(),3);
    auto erased = biHashMap.eraseTo("Two");
    EXPECT_TRUE(erased);
    EXPECT_EQ(biHashMap.size(),2);
    EXPECT_FALSE(biHashMap.containsFrom(2));
    EXPECT_FALSE(biHashMap.containsTo("Two"));
    auto notErased = biHashMap.eraseTo("NonExisting");
    EXPECT_FALSE(notErased);
    EXPECT_EQ(biHashMap.size(),2);
}

TEST_F(BidirectionalHashMapTest, erase){
    EXPECT_EQ(biHashMap.size(),3);
    auto erased = biHashMap.erase(3);
    EXPECT_TRUE(erased);
    EXPECT_EQ(biHashMap.size(),2);
    EXPECT_FALSE(biHashMap.contains(3));
    auto erased2 = biHashMap.erase("One");
    EXPECT_TRUE(erased2);
    EXPECT_EQ(biHashMap.size(),1);
    EXPECT_FALSE(biHashMap.contains("One"));
    auto notErased = biHashMap.erase(42);
    EXPECT_FALSE(notErased);
}

TEST_F(BidirectionalHashMapTest, insertPair){
    EXPECT_EQ(biHashMap.get(1).value(),"One");
    EXPECT_SIZE(biHashMap,3);
    biHashMap.insert(std::make_pair(1, "Uno"));
    EXPECT_EQ(biHashMap.get(1).value(),"Uno");
    EXPECT_SIZE(biHashMap,3);
    auto pair = std::make_pair(4, "Four");
    biHashMap.insert(pair);
    EXPECT_EQ(biHashMap.get(4).value(),"Four");
    EXPECT_SIZE(biHashMap,4);
}

TEST_F(BidirectionalHashMapTest, insert){
    EXPECT_EQ(biHashMap.size(),3);
    biHashMap.insert(4, "Four");
    EXPECT_EQ(biHashMap.size(),4);
    EXPECT_TRUE(biHashMap.contains(4));
    EXPECT_TRUE(biHashMap.contains("Four"));
    biHashMap.insert(4, "Cuatro");
    EXPECT_EQ(biHashMap.get(4).value(),"Cuatro");
    EXPECT_TRUE(biHashMap.contains(4));
    EXPECT_TRUE(biHashMap.contains("Cuatro"));
    EXPECT_FALSE(biHashMap.contains("Four"));
    EXPECT_EQ(biHashMap.size(),4);
}

TEST_F(BidirectionalHashMapTest, tryInsertValueType){
    auto result = biHashMap.try_insert(std::make_pair(0, "Zero"));
    EXPECT_TRUE(result);
    EXPECT_EQ(biHashMap.size(),4);
    auto duplicate = biHashMap.try_insert(std::make_pair(0, "Different"));
    EXPECT_FALSE(duplicate);
    EXPECT_EQ(biHashMap.size(),4);
    EXPECT_EQ(biHashMap.getTo(0).value(),"Zero");
}

TEST_F(BidirectionalHashMapTest, tryInsertFromTo){
    EXPECT_EQ(biHashMap.size(),3);
    auto result = biHashMap.try_insert(5, "Five");
    EXPECT_TRUE(result);
    EXPECT_EQ(biHashMap.size(),4);
    auto duplicate = biHashMap.try_insert(5, "Different");
    EXPECT_FALSE(duplicate);
    EXPECT_EQ(biHashMap.size(),4);
    EXPECT_EQ(biHashMap.getTo(5).value(),"Five");
}

TEST_F(BidirectionalHashMapTest, getTo) {
    auto val = biHashMap.getTo(2);
    EXPECT_VALUE(val);
    EXPECT_EQ(val.value(),"Two");
    auto empty = biHashMap.getTo(42);
    EXPECT_EMPTY(empty);
}

TEST_F(BidirectionalHashMapTest, getFrom) {
    auto val = biHashMap.getFrom("Three");
    EXPECT_VALUE(val);
    EXPECT_EQ(val.value(),3);
    auto empty = biHashMap.getFrom("NonExisting");
    EXPECT_EMPTY(empty);
}

TEST_F(BidirectionalHashMapTest, get){
    auto valFrom = biHashMap.get(2);
    EXPECT_VALUE(valFrom);
    EXPECT_EQ(valFrom.value(),"Two");
    auto valTo = biHashMap.get("Three");
    EXPECT_VALUE(valTo);
    EXPECT_EQ(valTo.value(),3);
    auto emptyFrom = biHashMap.get(42);
    EXPECT_EMPTY(emptyFrom);
    auto emptyTo = biHashMap.get("NonExisting");
    EXPECT_EMPTY(emptyTo);
}

