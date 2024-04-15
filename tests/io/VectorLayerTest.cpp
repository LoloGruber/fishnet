#include <gtest/gtest.h>
#include "VectorLayer.hpp"
#include <fishnet/Vec2D.hpp>
#include <list>
#include "Testutil.h"
#include <fishnet/PathHelper.h>
#include <fishnet/TemporaryDirectiory.h>

using namespace testutil;
using namespace fishnet;


class VectorLayerTest: public ::testing::Test {
protected:
    void SetUp() override {
        sampleLayer = VectorLayer<geometry::Polygon<double>>::read(pathToSample);
        pointLayer = VectorLayer<geometry::Vec2DReal>::empty(sampleLayer.getSpatialReference());
        pointLayer.addAllGeometry(points);
    }
    Shapefile pathToSample {util::PathHelper::projectDirectory() / std::filesystem::path("data/output/small_dataset/Punjab_Small.shp")};
    VectorLayer<geometry::Polygon<double>> sampleLayer = VectorLayer<geometry::Polygon<double>>::read(pathToSample);
    VectorLayer<geometry::Vec2DReal> pointLayer = VectorLayer<geometry::Vec2DReal>::empty(sampleLayer.getSpatialReference());
    Vec2DReal p1 = {0.5,1};
    Vec2DReal p2 = {3,-1};
    std::vector<Vec2DReal> points {p1,p2};
};


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

}

TEST_F(VectorLayerTest, clearFields) {
    
}

/*TEST_F(VectorLayerTest, addAttribute) {
    pointLayer.addField<const char *>("name");
    std::vector<size_t> ids;
    for(const auto & [id,geometry]: pointLayer.enumerateGeometries()) {
        if(geometry == p1){
            pointLayer.addAttribute(id,"name","p1");
        }
        if(geometry == p2) {
            pointLayer.addAttribute(id,"name","p2");
        }
        ids.push_back(id);
    }
    EXPECT_EQ(pointLayer.getAttribute<const char *>(ids[0],"name").value(),"p1");
    EXPECT_EQ(pointLayer.getAttribute<const char *>(ids[1],"name").value(),"p2");
    sampleLayer.addField<double>("area");
    for(const auto & [id,geom] : sampleLayer.enumerateGeometries()){
        EXPECT_TRUE(sampleLayer.addAttribute(id,"area",geom.area()));
    }
    for(const auto & [id,g] : sampleLayer.enumerateGeometries()){
        EXPECT_EQ(sampleLayer.getAttribute<double>(id,"area"),g.area());
    }
}

TEST_F(VectorLayerTest, getAttribute) {
    pointLayer.addField<int>("pid");
    int pid = 10;
    std::vector<size_t> ids;
    for(const auto & [id,p]:pointLayer.enumerateGeometries()){
        pointLayer.addAttribute(id,"pid",pid);
        ids.push_back(id);
        pid++;
    }
    EXPECT_EMPTY(pointLayer.getAttribute<double>(ids[0],"pid"));
    EXPECT_EQ(pointLayer.getAttribute<int>(ids[0],"pid").value(),10);
    EXPECT_EMPTY(pointLayer.getAttribute<int>(123456789,"pid"));
    EXPECT_EMPTY(pointLayer.getAttribute<int>(ids[0],"notexists"));
    TODO(); // add tests for specific getters
}

TEST_F(VectorLayerTest, removeAttribute) {
    pointLayer.addField<int>("pid");
    int pid = 10;
    std::vector<size_t> ids;
    for(const auto & [id,p]:pointLayer.enumerateGeometries()){
        pointLayer.addAttribute(id,"pid",pid);
        ids.push_back(id);
        pid++;
    }
    EXPECT_EQ(pointLayer.getAttribute<int>(ids[0],"pid").value(),10);
    EXPECT_EQ(pointLayer.getAttribute<int>(ids[1],"pid").value(),11);
    EXPECT_TRUE(pointLayer.removeAttribute(ids[0],"pid"));
    EXPECT_EMPTY(pointLayer.getAttribute<int>(ids[0],"pid"));
    EXPECT_EQ(pointLayer.getAttribute<int>(ids[1],"pid").value(),11);
    EXPECT_FALSE(pointLayer.removeAttribute(ids[1],"notexists"));
    EXPECT_FALSE(pointLayer.removeAttribute(123456789,"pid"));
    EXPECT_EMPTY(pointLayer.getAttribute<int>(ids[0],"pid"));
    EXPECT_EQ(pointLayer.getAttribute<int>(ids[1],"pid").value(),11);
    pointLayer.removeGeometry(p2);
    EXPECT_EMPTY(pointLayer.getAttribute<int>(ids[0],"pid"));
    EXPECT_EMPTY(pointLayer.getAttribute<int>(ids[1],"pid"));
}

TEST_F(VectorLayerTest, removeAttributeString) {
    pointLayer.addField<std::string>("str");
    for(const auto & [id,g]:pointLayer.enumerateGeometries()){
        EXPECT_TRUE(pointLayer.addAttribute(id,"str",std::to_string(Vec2DReal::type)));
    }
    std::vector<size_t> ids;
    for(const auto & [id,g]:pointLayer.enumerateGeometries()) {
        ids.push_back(id);
        EXPECT_EQ(pointLayer.getAttribute<std::string>(id,"str"),std::to_string(Vec2DReal::type));
    }
    EXPECT_TRUE(pointLayer.removeAttribute(ids[0],"str"));
    EXPECT_EMPTY(pointLayer.getAttribute<std::string>(ids[0],"str"));
    EXPECT_EQ(pointLayer.getAttribute<std::string>(ids[1],"str"),std::to_string(Vec2DReal::type));
    pointLayer.removeGeometry(p2);
    EXPECT_EMPTY(pointLayer.getAttribute<std::string>(ids[0],"str"));
    EXPECT_EMPTY(pointLayer.getAttribute<std::string>(ids[1],"str"));
}*/

TEST_F(VectorLayerTest, write) {
    util::TemporaryDirectory tmp {pathToSample.getPath().parent_path() / std::filesystem::path("tmp/")};
    Shapefile outputFile = {tmp.getDirectory() / std::filesystem::path(pathToSample.getPath().stem().string()+".shp")};
    EXPECT_FALSE(std::filesystem::exists(outputFile.getPath()));
    sampleLayer.write(outputFile);
    EXPECT_TRUE(std::filesystem::exists(outputFile.getPath()));
    EXPECT_UNSORTED_RANGE_EQ(sampleLayer.getGeometries(),VectorLayer<geometry::Polygon<double>>::read(outputFile).getGeometries());
    sampleLayer.write(outputFile);
    EXPECT_TRUE(std::filesystem::exists(outputFile.getPath()));
    EXPECT_TRUE(std::filesystem::exists(outputFile.incrementFileVersion().getPath()));
}

TEST_F(VectorLayerTest, overwrite) {
    util::TemporaryDirectory tmp {pathToSample.getPath().parent_path() / std::filesystem::path("tmp/")}; 
    Shapefile outputFile = {tmp.getDirectory() / std::filesystem::path(pathToSample.getPath().stem().string()+".shp")};  
    EXPECT_FALSE(std::filesystem::exists(outputFile.getPath()));
    sampleLayer.overwrite(outputFile);
    EXPECT_TRUE(std::filesystem::exists(outputFile.getPath()));
    sampleLayer.overwrite(outputFile);
    EXPECT_TRUE(std::filesystem::exists(outputFile.getPath()));
    EXPECT_FALSE(std::filesystem::exists(outputFile.incrementFileVersion().getPath())); 
}




