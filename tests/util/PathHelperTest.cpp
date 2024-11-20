#include <gtest/gtest.h>
#include <fishnet/PathHelper.h>
using namespace fishnet::util;
TEST(PathHelperTest, getCurrentPath) {
    EXPECT_EQ(PathHelper::getCurrentPath(),std::filesystem::current_path());
}

TEST(PathHelperTest, projectDirectory) {
    EXPECT_EQ(PathHelper::projectDirectory("fishnet"),PathHelper::absoluteCanonical(std::filesystem::path("../../../")));
}

TEST(PathHelperTest, absoluteCanonical){
    EXPECT_EQ(PathHelper::absoluteCanonical(PathHelper::projectDirectory() / std::filesystem::path("data/testing/../")),std::filesystem::path(PathHelper::projectDirectory()/std::filesystem::path("data")));
}

