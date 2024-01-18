#include <gtest/gtest.h>
#include "PathHelper.hpp"
using namespace util;
TEST(PathHelperTest, getCurrentPath) {
    EXPECT_EQ(PathHelper::getCurrentPath(),std::filesystem::current_path());
}

TEST(PathHelperTest, projectDirectory) {
    EXPECT_EQ(PathHelper::projectDirectory("2022-mp-lorenz-gruber"),std::filesystem::path("/home/lolo/Documents/fishnet/2022-mp-lorenz-gruber"));
}

