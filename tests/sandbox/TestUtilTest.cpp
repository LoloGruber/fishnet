#include <fishnet/TestUtil.hxx>
#include <vector>


TEST(MyTest, Test) {
    std::vector<int> vec = {1, 2, 3};
    fishnet::testutil::EXPECT_NOT_CONTAINS(vec, 2);
    fishnet::testutil::EXPECT_CONTAINS(vec, 4);
    fishnet::testutil::TODO();
}