#include <fishnet/TestUtil.hpp>
#include <fishnet/BidirectionalMap.hpp>

using namespace testutil;


class BidirectionalHashMultiMapTest: public ::testing::Test {
protected:
    void SetUp() override {
        biHashMultiMap = fishnet::util::BidirectionalHashMultiMap<size_t,std::string>();
        biHashMultiMap.insert({1,"One"});
        biHashMultiMap.insert({1,"Uno"});
        biHashMultiMap.insert({2,"Two"});
        biHashMultiMap.insert({2,"Dos"});
        biHashMultiMap.insert({3,"Three"});
    }  
    fishnet::util::BidirectionalHashMultiMap<size_t, std::string> biHashMultiMap;
};

TEST_F(BidirectionalHashMultiMapTest, initList) {
    std::initializer_list<std::pair<int,double>> initList {{1,1.0},{1,1.0},{2,2.0}};
    auto biMap = fishnet::util::BidirectionalHashMultiMap<int,double>(initList);
    EXPECT_SIZE(biMap,3);
}

TEST_F(BidirectionalHashMultiMapTest, getTo) {
    auto val = biHashMultiMap.getTo(3);
    EXPECT_VALUE(val);
    EXPECT_UNSORTED_RANGE_EQ(val.value(), std::vector<std::string>{"Three"});
    auto valFrom = biHashMultiMap.getTo(2);
    EXPECT_VALUE(valFrom);
    EXPECT_UNSORTED_RANGE_EQ(valFrom.value(), std::vector<std::string>{"Two", "Dos"});
    auto empty = biHashMultiMap.getFrom("NonExisting");
    EXPECT_EMPTY(empty);
}

TEST_F(BidirectionalHashMultiMapTest, getFrom) {
    auto val1 = biHashMultiMap.getFrom("One");
    EXPECT_VALUE(val1);
    EXPECT_UNSORTED_RANGE_EQ(val1.value(), std::vector<size_t>{1});
    auto val2 = biHashMultiMap.getFrom("Dos");
    EXPECT_VALUE(val2);
    EXPECT_UNSORTED_RANGE_EQ(val2.value(), std::vector<size_t>{2});
    auto empty = biHashMultiMap.getFrom("NonExisting");
    EXPECT_EMPTY(empty);
    biHashMultiMap.insert(5,"Uno");
    auto valUpdate = biHashMultiMap.getFrom("Uno");
    EXPECT_VALUE(valUpdate);
    EXPECT_UNSORTED_RANGE_EQ(valUpdate.value(), std::vector<size_t>{1, 5});
}

TEST_F(BidirectionalHashMultiMapTest, get) {
    auto val1 = biHashMultiMap.get(3);
    EXPECT_VALUE(val1);
    EXPECT_UNSORTED_RANGE_EQ(val1.value(), std::vector<std::string>{"Three"});
    auto val2 = biHashMultiMap.get("Dos");
    EXPECT_VALUE(val2);
    EXPECT_UNSORTED_RANGE_EQ(val2.value(), std::vector<size_t>{2});
    auto empty = biHashMultiMap.get("NonExisting");
    EXPECT_EMPTY(empty);
}