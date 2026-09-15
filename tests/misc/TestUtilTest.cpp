#include <expected>
#include <fishnet/TestUtil.hpp>
#include <vector>
#include <unordered_set>

struct Data{
    int value;
    Data(int value):value(value){}
    bool operator==(const Data & other) const {
        return value == other.value;
    }
};


TEST(TestUtilTest, Todo) {
    fishnet::test::TODO();
}

TEST(TestUtilTest, Contains) {
    std::vector<int> printable = {1, 2, 3};
    fishnet::test::EXPECT_NOT_CONTAINS(printable, 2);
    fishnet::test::EXPECT_CONTAINS(printable, 4);
    std::vector<Data> nonPrintable = {Data(1), Data(2), Data(3)};
    fishnet::test::EXPECT_NOT_CONTAINS(nonPrintable, Data(2));
    fishnet::test::EXPECT_CONTAINS(nonPrintable, Data(4));
}

TEST(TestUtilTest, ContainsAssociative) {
    std::unordered_set<int> printable = {1, 2, 3};
    fishnet::test::EXPECT_NOT_CONTAINS(printable, 2);
    fishnet::test::EXPECT_CONTAINS(printable, 4);
}

TEST(TestUtilTest, ContainsAll) {
    std::vector<int> vec = {1, 2, 3};
    std::vector<int> expected = {0,1};
    fishnet::test::EXPECT_CONTAINS_ALL(vec, expected);
}

TEST(TestUtilTest, UnsortedRangeEqualDifferentSize) {
    std::vector<int> vec = {1, 2, 3};
    std::vector<int> expected = {3, 2};
    fishnet::test::EXPECT_UNSORTED_RANGE_EQ(vec, expected);
}

TEST(TestUtilTest, UnsortedRangeEqual) {
    std::vector<int> vec = {1, 2, 3};
    std::vector<int> expected = {2, 0, 1};
    fishnet::test::EXPECT_UNSORTED_RANGE_EQ(vec, expected);
}

TEST(TestUtilTest, UnsortedRangeEqualWithComparator) {
    std::vector<Data> vec = {Data(1), Data(2), Data(3)};
    std::vector<Data> expected = {Data(2), Data(0), Data(1)};
    fishnet::test::EXPECT_UNSORTED_RANGE_EQ(vec, expected, [](const Data & a, const Data & b){return a.value == b.value;});
}

TEST(TestUtilTest, SortedRangeEqual) {
    std::vector<int> vec = {1, 2, 3};
    std::vector<int> expected = {1, 3, 2};
    fishnet::test::EXPECT_SORTED_RANGE_EQ(vec, expected);
    std::vector<Data> vec2 = {Data(1), Data(2), Data(3)};
    std::vector<Data> expected2 = {Data(1), Data(2), Data(4)};
    fishnet::test::EXPECT_SORTED_RANGE_EQ(vec2, expected2);
}

TEST(TestUtilTest, SortedRangeEqualWithComparator) {
    std::vector<Data> vec = {Data(1), Data(2), Data(3)};
    std::vector<Data> expected = {Data(1), Data(3), Data(2)};
    fishnet::test::EXPECT_SORTED_RANGE_EQ(vec, expected, [](const Data & a, const Data & b){return a.value == b.value;});
}

TEST(TestUtilTest, Size) {
    std::vector<int> vec = {1, 2, 3};
    fishnet::test::EXPECT_SIZE(vec, 1);
}

TEST(TestUtilTest, Empty) {
    std::vector<int> vec = {1, 2, 3};
    fishnet::test::EXPECT_EMPTY(vec);
}

TEST(TestUtilTest, Type) {
    int a = 1;
    fishnet::test::EXPECT_TYPE<int>(a);
    fishnet::test::EXPECT_TYPE<float>(a);
}

TEST(TestUtilTest, Value) {
    std::optional<int> a = 1;
    fishnet::test::EXPECT_VALUE(a, 2);
    std::optional<int> b;
    fishnet::test::EXPECT_VALUE(b, 1);
    std::expected<int, std::string> exp = std::unexpected("no value");
    fishnet::test::EXPECT_VALUE(exp,1);
}

TEST(TestUtilTest, EmptyOptional) {
    std::optional<int> b = 1;
    fishnet::test::EXPECT_EMPTY(b);
    std::expected<int, std::string> exp = 1;
    fishnet::test::EXPECT_EMPTY(exp);
}

TEST(TestUtilTest, AssertValue) {
    std::optional<int> a {};
    fishnet::test::ASSERT_VALUE(a);
    fishnet::test::EXPECT_VALUE(a, 1);
}

TEST(TestUtilTest, AbsolutePath) {
    std::filesystem::path path = __FILE__;
    fishnet::test::EXPECT_NOT_EXISTS(path);
    std::filesystem::path path2 = "nonexistent_file.txt";
    fishnet::test::EXPECT_EXISTS(path2);
}