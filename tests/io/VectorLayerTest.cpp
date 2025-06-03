#include <gtest/gtest.h>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/Vec2D.hpp>
#include <list>
#include "Testutil.h"
#include <fishnet/PathHelper.h>
#include <fishnet/TemporaryDirectiory.h>
#include <fishnet/VectorIO.hpp>

using namespace testutil;
using namespace fishnet;
using namespace fishnet::geometry;

class VectorLayerTest: public ::testing::Test {
protected:
    void SetUp() override {
        sampleLayer = VectorLayer<geometry::Polygon<double>>::read(pathToSample);
        pointLayer = VectorLayer<geometry::Vec2DReal>::empty(sampleLayer.getSpatialReference());
        pointLayer.addAllGeometry(points);
    }
    Shapefile pathToSample {util::PathHelper::projectDirectory() / std::filesystem::path("data/testing/Punjab_Small/Punjab_Small.shp")};
    VectorLayer<geometry::Polygon<double>> sampleLayer = VectorLayer<geometry::Polygon<double>>::read(pathToSample);
    VectorLayer<geometry::Vec2DReal> pointLayer = VectorLayer<geometry::Vec2DReal>::empty(sampleLayer.getSpatialReference());
    Vec2DReal p1 = {0.5,1};
    Vec2DReal p2 = {3,-1};
    std::vector<Vec2DReal> points {p1,p2};
};

TEST_F(VectorLayerTest, read){
    EXPECT_TRUE(std::filesystem::exists(pathToSample.getPath())) << "Path to testcase sample does not exist: " << pathToSample.getPath();
    auto layer = VectorIO::read<geometry::Polygon<double>>(pathToSample);
    EXPECT_TYPE<VectorLayer<geometry::Polygon<double>>>(layer);
    EXPECT_FALSE(layer.isEmpty()) << "Layer should not be empty after successful read";
}

TEST_F(VectorLayerTest, init){
    EXPECT_TRUE(std::filesystem::exists(pathToSample.getPath()));
    auto layer = VectorLayer<fishnet::geometry::Polygon<double>>::read(pathToSample);

    Shapefile notExistsFile {util::PathHelper::projectDirectory() / std::filesystem::path("tests/io/does_not_exists.shp")};
    EXPECT_ANY_THROW(VectorLayer<fishnet::geometry::Polygon<double>>::read(notExistsFile));
    EXPECT_NO_THROW(VectorLayer<geometry::Polygon<double>>::empty(layer.getSpatialReference()));
    EXPECT_FALSE(notExistsFile.exists());
    auto empty = VectorLayer<fishnet::geometry::Polygon<double>>::empty(layer.getSpatialReference());
    EXPECT_EMPTY(empty.getGeometries());
    EXPECT_EMPTY(empty.getFeatures());
    EXPECT_NO_THROW(VectorLayer<geometry::Polygon<double>>::empty(layer.getSpatialReference()));
}

TEST_F(VectorLayerTest,getGeometries) {
    EXPECT_NO_FATAL_FAILURE(sampleLayer.getGeometries());
    EXPECT_SIZE(pointLayer.getGeometries(),points.size());
    EXPECT_CONTAINS(pointLayer.getGeometries(),p1);
    EXPECT_CONTAINS(pointLayer.getGeometries(),p2);
}

TEST_F(VectorLayerTest, getFeatures) {
    EXPECT_SIZE(pointLayer.getFeatures(),points.size());
    auto testFeature = Feature(p1);
    EXPECT_CONTAINS(pointLayer.getFeatures(), testFeature);
}

TEST_F(VectorLayerTest, addGeometry){
    pointLayer.addGeometry(Vec2DReal(0,0));
    EXPECT_CONTAINS(pointLayer.getGeometries(),Vec2DReal(0,0));
    EXPECT_SIZE(pointLayer.getGeometries(),3);
    EXPECT_SIZE(pointLayer.getFeatures(), 3);
}

TEST_F(VectorLayerTest, addAllGeometry) {
    auto layer = VectorLayer<geometry::Vec2DStd>::empty(pointLayer.getSpatialReference());
    EXPECT_EMPTY(layer.getGeometries());
    layer.addAllGeometry(points);
    EXPECT_SIZE(layer.getGeometries(),2);
    EXPECT_SIZE(layer.getFeatures(), 2);
}

TEST_F(VectorLayerTest, containsGeometry) {
    pointLayer.addGeometry(Vec2DReal(0,0));
    Feature<Vec2DReal> emptyFeatureSameGeometry{Vec2DReal(0, 0)};
    EXPECT_TRUE(pointLayer.containsGeometry(Vec2DReal(0,0)));
    EXPECT_TRUE(pointLayer.containsFeature(emptyFeatureSameGeometry));
    EXPECT_FALSE(pointLayer.containsGeometry(Vec2DReal(-1,-1)));
}

TEST_F(VectorLayerTest, removeGeometry) {
    pointLayer.removeGeometry(p1);
    EXPECT_SIZE(pointLayer.getGeometries(),1);
    EXPECT_CONTAINS(pointLayer.getGeometries(),p2);
    EXPECT_NOT_CONTAINS(pointLayer.getGeometries(),p1);
}

TEST_F(VectorLayerTest, getSpatialReference) {
    const char * wktSpatRef = "GEODCRS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",ELLIPSOID[\"WGS 84\", 6378137, 298.257223563, LENGTHUNIT[\"metre\", 1]]],CS[ellipsoidal, 2],AXIS[\"Latitude (lat)\", north, ORDER[1]],AXIS[\"Longitude (lon)\", east, ORDER[2]],ANGLEUNIT[\"degree\", 0.0174532925199433]]";
    OGRSpatialReference spatRef = OGRSpatialReference(wktSpatRef);
    auto points = VectorLayer<Vec2DStd>::empty(spatRef);
    EXPECT_EQ(spatRef.GetName(),points.getSpatialReference().GetName());
}

TEST_F(VectorLayerTest, setSpatialReference) {
    const char * wktSpatRef = "GEODCRS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",ELLIPSOID[\"WGS 84\", 6378137, 298.257223563, LENGTHUNIT[\"metre\", 1]]],CS[ellipsoidal, 2],AXIS[\"Latitude (lat)\", north, ORDER[1]],AXIS[\"Longitude (lon)\", east, ORDER[2]],ANGLEUNIT[\"degree\", 0.0174532925199433]]";
    OGRSpatialReference spatRef = OGRSpatialReference(wktSpatRef);
    pointLayer.setSpatialReference(spatRef);
    EXPECT_EQ(spatRef.GetName(), pointLayer.getSpatialReference().GetName());
}

TEST_F(VectorLayerTest, addFieldInvalid){
    std::string longFieldName = "VeryVeryLongFieldName";
    EXPECT_EMPTY(pointLayer.addIntegerField(longFieldName));
    std::string duplicatedName = "myField";
    EXPECT_VALUE(pointLayer.addTextField(duplicatedName));
    EXPECT_EMPTY(pointLayer.addSizeField(duplicatedName));
}


TEST_F(VectorLayerTest, addFieldValid) {
    auto id = pointLayer.addField<int>("id");
    EXPECT_VALUE(id);
    auto length = pointLayer.addDoubleField("length");
    EXPECT_VALUE(length);
    int counter = 0;
    for( auto & feature : pointLayer.getFeatures()){
        EXPECT_TRUE(feature.addAttribute(id.value(), counter++));
        EXPECT_TRUE(feature.addAttribute(length.value(), feature.getGeometry().length()));
    }
    Feature ofP1{p1};
    Feature ofP2{p2};
    ofP1.addAttribute(id.value(), 0);
    ofP1.addAttribute(length.value(), p1.length());
    ofP2.addAttribute(id.value(), 1);
    ofP2.addAttribute(length.value(), p2.length());
    std::vector<Feature<geometry::Vec2DReal >> expected = {ofP1, ofP2};
    EXPECT_UNSORTED_RANGE_EQ(pointLayer.getFeatures(), expected);

}

TEST_F(VectorLayerTest, hasField) {
    std::string fieldName = "myField";
    EXPECT_VALUE(pointLayer.addDoubleField(fieldName));
    EXPECT_TRUE(pointLayer.hasField(fieldName));
    EXPECT_FALSE(pointLayer.hasField("other"));
}

TEST_F(VectorLayerTest, removeField) {
    std::string areaField = "area";
    EXPECT_VALUE(pointLayer.addDoubleField(areaField));
    EXPECT_TRUE(pointLayer.hasField(areaField));
    EXPECT_NO_FATAL_FAILURE(pointLayer.removeField("not-existing"));
    EXPECT_NO_FATAL_FAILURE(pointLayer.removeField(areaField));
    EXPECT_FALSE(pointLayer.hasField(areaField));
}

TEST_F(VectorLayerTest, clearFields) {
    std::string field1 = "f1";
    std::string field2 = "f2";
    EXPECT_VALUE(pointLayer.addIntegerField(field1));
    EXPECT_VALUE(pointLayer.addDoubleField(field2));
    EXPECT_TRUE(pointLayer.hasField(field1));
    EXPECT_TRUE(pointLayer.hasField(field2));
    EXPECT_NO_FATAL_FAILURE(pointLayer.clearFields());
    EXPECT_FALSE(pointLayer.hasField(field1));
    EXPECT_FALSE(pointLayer.hasField(field2));
}

TEST_F(VectorLayerTest, getField) {
    std::string intField = "amount";
    using Feature_t = typename decltype(pointLayer)::feature_type;
    Feature_t pointFeature {Vec2DReal(4,2)};
    auto amountField = pointLayer.addIntegerField(intField);
    EXPECT_VALUE(amountField);
    int value = 10;
    pointFeature.addAttribute(amountField.value(),value);
    EXPECT_EMPTY(pointLayer.getField<double>("other"));
    EXPECT_EMPTY(pointLayer.getField<std::string>(intField));
    auto fieldAfterGet = pointLayer.getField<int>(intField);
    EXPECT_VALUE(fieldAfterGet);
    EXPECT_EQ(pointFeature.getAttribute(fieldAfterGet.value()),value);
}

TEST_F(VectorLayerTest, write) {
    util::AutomaticTemporaryDirectory tmp {};
    Shapefile outputFile = {tmp / std::filesystem::path(pathToSample.getPath().stem().string()+".shp")};
    EXPECT_FALSE(std::filesystem::exists(outputFile.getPath()));
    EXPECT_NO_FATAL_FAILURE(outputFile = VectorIO::write(sampleLayer, outputFile));
    sampleLayer.write(outputFile);
    EXPECT_TRUE(std::filesystem::exists(outputFile.getPath()));
    EXPECT_UNSORTED_RANGE_EQ(sampleLayer.getGeometries(),VectorLayer<geometry::Polygon<double>>::read(outputFile).getGeometries());
    sampleLayer.write(outputFile);
    EXPECT_TRUE(std::filesystem::exists(outputFile.getPath()));
    EXPECT_TRUE(std::filesystem::exists(outputFile.incrementFileVersion().getPath()));
}

TEST_F(VectorLayerTest, overwrite) {
    util::AutomaticTemporaryDirectory tmp {}; 
    Shapefile outputFile = {tmp / std::filesystem::path(pathToSample.getPath().stem().string()+".shp")};  
    EXPECT_FALSE(std::filesystem::exists(outputFile.getPath()));
    sampleLayer.overwrite(outputFile);
    EXPECT_TRUE(std::filesystem::exists(outputFile.getPath()));
    sampleLayer.overwrite(outputFile);
    EXPECT_TRUE(std::filesystem::exists(outputFile.getPath()));
    EXPECT_FALSE(std::filesystem::exists(outputFile.incrementFileVersion().getPath())); 
}




