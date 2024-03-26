//
// Created by lolo on 3/26/24.
//
#include "Feature.hpp"
#include "FieldDefinitionTestFactory.hpp"
#include "Vec2D.hpp"
#include "Line.hpp"
#include <gtest/gtest.h>
#include "Testutil.h"
using namespace fishnet;
using namespace testutil;

TEST(FeatureTest, init) {
    EXPECT_NO_FATAL_FAILURE(Feature(geometry::Vec2DStd(0,0)));
    EXPECT_NO_FATAL_FAILURE(Feature(geometry::Line({0,0},{1,1})));
}

TEST(FeatureTest, getGeometry){
    auto vec = geometry::Vec2DStd(0, 0);
    Feature f{vec};
    EXPECT_EQ(vec,f.getGeometry());
}

TEST(FeatureTest, addAttribute) {
    Feature f{geometry::Vec2DStd(-1, 1.41)};
    auto field = FieldDefinitionTestFactory<double>::createField("testField");
    EXPECT_TRUE(f.addAttribute(field, 3.14));
    EXPECT_FALSE(f.addAttribute(field,2.0));
    EXPECT_VALUE(f.getAttribute(field));
    EXPECT_DOUBLE_EQ(3.14,f.getAttribute(field).value());
}

TEST(FeatureTest, setAttribute) {
    Feature f{geometry::Vec2DStd(0, 0)};
    auto length = FieldDefinitionTestFactory<double>::createField("length");
    auto name = FieldDefinitionTestFactory<std::string>::createField("name");
    double value = 10000.0;
    EXPECT_NO_FATAL_FAILURE(f.setAttribute(length, value));
    EXPECT_NO_FATAL_FAILURE(f.addAttribute(name,"X-Axis"));
    double newValue = 42.042;
    EXPECT_EQ(value,f.getAttribute(length).value());
    EXPECT_FALSE(f.addAttribute(length,newValue));
    EXPECT_EQ("X-Axis",f.getAttribute(name).value());
    EXPECT_EQ(value,f.getAttribute(length).value());
    EXPECT_NO_FATAL_FAILURE(f.setAttribute(length,newValue));
    EXPECT_EQ(newValue,f.getAttribute(length).value());
}

TEST(FeatureTest, hasAttribute){
    Feature f{geometry::Vec2DStd(0, 0)};
    auto length = FieldDefinitionTestFactory<double>::createField("length");
    EXPECT_FALSE(f.hasAttribute(length));
    EXPECT_TRUE(f.addAttribute(length,1));
    EXPECT_TRUE(f.hasAttribute(length));
    auto amount = FieldDefinitionTestFactory<int>::createField("amount");
    EXPECT_FALSE(f.hasAttribute(amount));
    EXPECT_NO_FATAL_FAILURE(f.setAttribute(amount,2));
    EXPECT_TRUE(f.hasAttribute(amount));
}

TEST(FeatureTest, getAttribute) {
    TODO();
}
