#include <gtest/gtest.h>
#include <fishnet/Shapefile.hpp>
#include <fishnet/TemporaryDirectiory.h>
#include <fishnet/PathHelper.h>
using namespace fishnet;

static const std::filesystem::path example {util::PathHelper::projectDirectory() / std::filesystem::path("data/testing/Punjab_Small/Punjab_Small.shp")};
TEST(ShapefileTest, init){
    Shapefile shp {example};
    EXPECT_EQ(shp.getPath(),example);
    EXPECT_ANY_THROW(Shapefile("/home/lolo/Documents/fishnet/2022-mp-lorenz-gruber/tests/io/ShapefileTest.cpp")); // not a shapefile
}

TEST(ShapefileTest, exists){
    EXPECT_TRUE(Shapefile(example).exists());
    EXPECT_FALSE(Shapefile("/home/lolo/does_not_exists.shp").exists());
}

TEST(ShapefileTest, copy){
    auto tmp = util::TemporaryDirectory("/home/lolo/tmp/");
    std::string filename = "myShape";
    Shapefile shp {example};
    shp.copy(tmp.getDirectory(),filename);
    EXPECT_TRUE(std::filesystem::exists(tmp.getDirectory() / (std::filesystem::path(filename+".shp"))));
}

TEST(ShapefileTest, incrementFileVersion) {
    Shapefile shp {example};
    auto incremented = shp.incrementFileVersion();
    EXPECT_EQ(incremented.getPath().stem().string(),example.stem().string()+"_1");
    EXPECT_EQ(incremented.incrementFileVersion().getPath().stem().string(),example.stem().string()+"_2");
    EXPECT_EQ(Shapefile("/test1.shp").incrementFileVersion().getPath().stem().string(),std::string("test1_1")); //not incrementing last number, since not a version number
}