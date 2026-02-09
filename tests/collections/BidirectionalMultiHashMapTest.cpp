#include <fishnet/TestUtil.hpp>
#include <fishnet/BidirectionalMap.hpp>

using namespace testutil;


class BidirectionalMultiHashMapTest: public ::testing::Test {
protected:
    void SetUp() override {
        biHashMultiMap = fishnet::util::BidirectionalMultiHashMap<size_t,std::string>();
        biHashMultiMap.insert(1,"One");
        biHashMultiMap.insert(1,"Uno");
        biHashMultiMap.insert(2,"Two");
        biHashMultiMap.insert(2,"Dos");
        biHashMultiMap.insert(3,"Three");
    }  
    fishnet::util::BidirectionalMultiHashMap<size_t, std::string> biHashMultiMap;
};

TEST_F(BidirectionalMultiHashMapTest, initList) {
    std::initializer_list<std::pair<int,double>> initList {{1,1.0},{1,1.0},{2,2.0}};
    auto biMap = fishnet::util::BidirectionalMultiHashMap<int,double>(initList);
    EXPECT_SIZE(biMap,3);
}

TEST_F(BidirectionalMultiHashMapTest, getTo) {
    auto val = biHashMultiMap.getTo(3);
    static_assert(std::same_as<std::ranges::range_value_t<decltype(val)>, std::string>);
    EXPECT_UNSORTED_RANGE_EQ(val, std::vector<std::string>{"Three"});
    auto valFrom = biHashMultiMap.getTo(2);
    EXPECT_UNSORTED_RANGE_EQ(valFrom, std::vector<std::string>{"Two", "Dos"});
    auto empty = biHashMultiMap.getFrom("NonExisting");
    EXPECT_EMPTY(empty);
    for(auto & value: biHashMultiMap.getTo(2)) {
        value = "Zwei";
    }
    EXPECT_UNSORTED_RANGE_EQ(biHashMultiMap.getTo(2), std::vector<std::string>{"Zwei", "Zwei"});
}

TEST_F(BidirectionalMultiHashMapTest, getFrom) {
    auto val1 = biHashMultiMap.getFrom("One");
    static_assert(std::same_as<std::ranges::range_value_t<decltype(val1)>, size_t>);
    EXPECT_UNSORTED_RANGE_EQ(val1, std::vector<size_t>{1});
    auto val2 = biHashMultiMap.getFrom("Dos");
    EXPECT_UNSORTED_RANGE_EQ(val2, std::vector<size_t>{2});
    auto empty = biHashMultiMap.getFrom("NonExisting");
    EXPECT_EMPTY(empty);
    biHashMultiMap.insert(5,"Uno");
    auto valUpdate = biHashMultiMap.getFrom("Uno");
    EXPECT_UNSORTED_RANGE_EQ(valUpdate, std::vector<size_t>{1, 5});
    for(auto & value: biHashMultiMap.getFrom("Uno")) {
        value = 10;
    }
    EXPECT_UNSORTED_RANGE_EQ(biHashMultiMap.getFrom("Uno"), std::vector<size_t>{10, 10});
}

TEST_F(BidirectionalMultiHashMapTest, get) {
    auto val1 = biHashMultiMap.get(3);
    EXPECT_UNSORTED_RANGE_EQ(val1, std::vector<std::string>{"Three"});
    auto val2 = biHashMultiMap.get("Dos");
    EXPECT_UNSORTED_RANGE_EQ(val2, std::vector<size_t>{2});
    auto empty = biHashMultiMap.get("NonExisting");
    EXPECT_EMPTY(empty);
    biHashMultiMap.erase(2);
    auto emptyAfterErase = biHashMultiMap.get(2);
    auto emptyInverseAfterErase = biHashMultiMap.get("Two");
    EXPECT_EMPTY(emptyAfterErase);
    EXPECT_EMPTY(emptyInverseAfterErase);
}