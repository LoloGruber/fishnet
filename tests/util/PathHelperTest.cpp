#include <gtest/gtest.h>
#include <fishnet/PathHelper.h>
using namespace fishnet::util;
TEST(PathHelperTest, getCurrentPath) {
    EXPECT_EQ(PathHelper::getCurrentPath(),std::filesystem::current_path());
}

TEST(PathHelperTest, projectDirectory) {
    EXPECT_EQ(PathHelper::projectDirectory("fishnet"),std::filesystem::path("/home/lolo/Documents/fishnet"));
}

