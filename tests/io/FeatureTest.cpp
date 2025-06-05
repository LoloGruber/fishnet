//
// Created by lolo on 3/26/24.
//
#include <fishnet/Feature.hpp>
#include "FieldDefinitionTestFactory.hpp"
#include <fishnet/Vec2D.hpp>
#include <fishnet/Line.hpp>
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
    EXPECT_TRUE(f.addAttribute(length,1.0));
    EXPECT_TRUE(f.hasAttribute(length));
    auto amount = FieldDefinitionTestFactory<int>::createField("amount");
    EXPECT_FALSE(f.hasAttribute(amount));
    EXPECT_NO_FATAL_FAILURE(f.setAttribute(amount,2));
    EXPECT_TRUE(f.hasAttribute(amount));
}

TEST(FeatureTest, getAttribute) {
    Feature f{geometry::Vec2DStd(0, 0)};
    auto myField = FieldDefinitionTestFactory<int>::createField("amount");
    EXPECT_EMPTY(f.getAttribute(myField));
    f.addAttribute(myField, 1);
    EXPECT_VALUE(f.getAttribute(myField));
}

TEST(FeatureTest, removeAttribute) {
    Feature f{geometry::Vec2DStd(0, 0)};
    auto myField = FieldDefinitionTestFactory<int>::createField("amount");
    f.addAttribute(myField, 0);
    EXPECT_TRUE(f.hasAttribute(myField));
    auto otherField = FieldDefinitionTestFactory<double>::createField("unused");
    EXPECT_NO_FATAL_FAILURE(f.removeAttribute(otherField));
    EXPECT_TRUE(f.hasAttribute(myField));
    EXPECT_NO_FATAL_FAILURE(f.removeAttribute(myField));
    EXPECT_FALSE(f.hasAttribute(myField));
    EXPECT_FALSE(f.hasAttribute(otherField));
}

TEST(FeatureTest, equality) {
    Feature lhs{geometry::Vec2DReal(0, 0)};
    Feature rhs{geometry::Vec2DReal(0, 0)};
    EXPECT_EQ(lhs,rhs);
    auto myField = FieldDefinitionTestFactory<int>::createField("amount");
    lhs.addAttribute(myField, 1);
    EXPECT_NE(lhs,rhs);
    auto otherField = FieldDefinitionTestFactory<long>::createField("different Field");
    rhs.addAttribute(otherField, 1); // Same value but different field
    EXPECT_NE(lhs,rhs);
    Feature copyOfLhs = lhs;
    EXPECT_EQ(lhs,copyOfLhs);
    lhs.setAttribute(myField, 2); // Same field, different value
    EXPECT_NE(lhs,copyOfLhs);
}

TEST(FeatureTest, copyAttributes) {
    Feature f1{geometry::Vec2DStd(0, 0)};
    Feature f2{geometry::Vec2DStd(1, 1)};
    auto myField = FieldDefinitionTestFactory<int>::createField("amount");
    f1.addAttribute(myField, 42);
    EXPECT_TRUE(f1.hasAttribute(myField));
    EXPECT_FALSE(f2.hasAttribute(myField));
    EXPECT_EMPTY(f2.getAttribute(myField));
    f2.copyAttributes(f1);
    EXPECT_TRUE(f2.hasAttribute(myField));
    EXPECT_EQ(f2.getAttribute(myField).value(), 42);
    Feature f3 {geometry::Segment<double>({0,0},{1,1})};
    f3.copyAttributes(f2);
    EXPECT_TRUE(f3.hasAttribute(myField));
    EXPECT_EQ(f3.getAttribute(myField).value(), 42);
}
