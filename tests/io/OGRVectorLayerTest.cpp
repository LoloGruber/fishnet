#include <gtest/gtest.h>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/TestUtil.hpp>
#include <fishnet/PathHelper.h>
#include <fishnet/TemporaryDirectiory.h>
#include <fishnet/VectorIO.hpp>
#include <fishnet/OGRGeometryAdapter.hpp>

using namespace fishnet::test;
using namespace fishnet;
using namespace fishnet::geometry;

/**
 * @brief A VectorLayer whose geometries are backed by the OGR geometries of the data source
 *
 * The sample is a polygon layer, so reading it as OGRMultiPolygonAdapter exercises the branch
 * lifting a lone polygon into a one part multi-polygon, and reading a multi-polygon layer back as
 * OGRPolygonAdapter exercises the opposite one.
 */
class OGRBackedVectorLayerTest: public ::testing::Test {
protected:
    Shapefile pathToSample {util::PathHelper::projectDirectory() / std::filesystem::path("data/testing/regions/Corvara_Small_Preprocessed.shp")};
    VectorLayer<OGRPolygonAdapter> sourceLayer = VectorIO::read<OGRPolygonAdapter>(pathToSample);
};

TEST_F(OGRBackedVectorLayerTest, readAsPolygonAdapter){
    auto layer = VectorIO::read<OGRPolygonAdapter>(pathToSample);
    EXPECT_FALSE(layer.isEmpty());
    ASSERT_EQ(sourceLayer.size(), layer.size());

    auto native = sourceLayer.getGeometries();
    auto adapted = layer.getGeometries();
    auto nativeIt = std::ranges::begin(native);
    auto adaptedIt = std::ranges::begin(adapted);
    for(; nativeIt != std::ranges::end(native); ++nativeIt, ++adaptedIt){
        // the native copy of a geometry has to describe the same shape as its adapter
        EXPECT_NEAR((*nativeIt).toNative().area(), (*adaptedIt).area(), 1e-6);
    }
}

TEST_F(OGRBackedVectorLayerTest, readPolygonLayerAsMultiPolygonAdapter){
    auto layer = VectorIO::read<OGRMultiPolygonAdapter>(pathToSample);
    ASSERT_EQ(sourceLayer.size(), layer.size());
    for(const auto & multiPolygon : layer.getGeometries()){
        EXPECT_EQ(1u, multiPolygon.size()) << "every polygon of the source becomes a one part multi-polygon";
    }
}

TEST_F(OGRBackedVectorLayerTest, roundTrip){
    util::AutomaticTemporaryDirectory tmp {};
    auto layer = VectorIO::read<OGRPolygonAdapter>(pathToSample);
    Shapefile outputFile = Shapefile{tmp / std::filesystem::path(pathToSample.getPath().stem().string()+".shp")};

    EXPECT_NO_FATAL_FAILURE(outputFile = VectorIO::write(layer, outputFile));
    ASSERT_TRUE(std::filesystem::exists(outputFile.getPath()));
    EXPECT_UNSORTED_RANGE_EQ(layer.getGeometries(), VectorIO::read<OGRPolygonAdapter>(outputFile).getGeometries());
}

TEST_F(OGRBackedVectorLayerTest, singlePartMultiPolygonIsReadAsPolygon){
    util::AutomaticTemporaryDirectory tmp {};
    auto multiPolygonLayer = VectorIO::read<OGRMultiPolygonAdapter>(pathToSample);
    Shapefile outputFile = Shapefile{tmp / std::filesystem::path("multi.shp")};
    outputFile = VectorIO::write(multiPolygonLayer, outputFile);

    // every feature is a multi-polygon of exactly one part, so all of them have to survive the
    // downcast back into a polygon typed layer
    auto readBack = VectorIO::read<OGRPolygonAdapter>(outputFile);
    EXPECT_EQ(multiPolygonLayer.size(), readBack.size());
}

TEST_F(OGRBackedVectorLayerTest, attributesSurviveTheRoundTrip){
    util::AutomaticTemporaryDirectory tmp {};
    auto layer = VectorIO::read<OGRPolygonAdapter>(pathToSample);
    auto field = layer.addIntegerField("marker");
    ASSERT_TRUE(field.has_value());
    int marker = 0;
    for(auto & feature : layer.getFeatures()){
        feature.setAttribute(field.value(), marker++);
    }

    Shapefile outputFile = Shapefile{tmp / std::filesystem::path("attributed.shp")};
    outputFile = VectorIO::write(layer, outputFile);
    auto readBack = VectorIO::read<OGRPolygonAdapter>(outputFile);

    auto readField = readBack.getIntegerField("marker");
    ASSERT_TRUE(readField.has_value());
    ASSERT_EQ(layer.size(), readBack.size());
    int expected = 0;
    for(const auto & feature : readBack.getFeatures()){
        auto value = feature.getAttribute(readField.value());
        ASSERT_TRUE(value.has_value());
        EXPECT_EQ(expected++, value.value());
    }
}

TEST_F(OGRBackedVectorLayerTest, readsTypeErasedByDefault){
    auto layer = VectorIO::read(pathToSample); // no geometry type named
    EXPECT_TYPE<VectorLayer<OGRGeometryAdapter>>(layer);
    ASSERT_EQ(sourceLayer.size(), layer.size()) << "a type erased read keeps every geometry of the source";
    for(const auto & geometry : layer.getGeometries()){
        EXPECT_TRUE(geometry.isPolygon());
    }
}

TEST_F(OGRBackedVectorLayerTest, narrowsATypeErasedLayer){
    auto erased = VectorIO::read(pathToSample);
    auto polygons = VectorIO::narrow<OGRPolygonAdapter>(erased);
    ASSERT_EQ(erased.size(), polygons.size());
    EXPECT_UNSORTED_RANGE_EQ(sourceLayer.getGeometries(), polygons.getGeometries());
}

TEST_F(OGRBackedVectorLayerTest, narrowingCarriesFieldsAndAttributes){
    auto erased = VectorIO::read(pathToSample);
    auto field = erased.addIntegerField("marker");
    ASSERT_TRUE(field.has_value());
    int marker = 0;
    for(auto & feature : erased.getFeatures()){
        feature.setAttribute(field.value(), marker++);
    }

    auto polygons = VectorIO::narrow<OGRPolygonAdapter>(erased);
    ASSERT_TRUE(polygons.hasField("marker"));
    auto narrowedField = polygons.getIntegerField("marker");
    ASSERT_TRUE(narrowedField.has_value());
    int expected = 0;
    for(const auto & feature : polygons.getFeatures()){
        auto value = feature.getAttribute(narrowedField.value());
        ASSERT_TRUE(value.has_value());
        EXPECT_EQ(expected++, value.value());
    }
}

TEST_F(OGRBackedVectorLayerTest, nativeCopyYieldsFishnetGeometries){
    auto native = VectorIO::nativeCopy(sourceLayer);
    EXPECT_TYPE<VectorLayer<Polygon<double>>>(native);
    ASSERT_EQ(sourceLayer.size(), native.size());

    auto adapted = sourceLayer.getGeometries();
    auto converted = native.getGeometries();
    auto adaptedIt = std::ranges::begin(adapted);
    auto convertedIt = std::ranges::begin(converted);
    for(; adaptedIt != std::ranges::end(adapted); ++adaptedIt, ++convertedIt){
        EXPECT_NEAR((*adaptedIt).area(), (*convertedIt).area(), 1e-6);
    }
}

TEST_F(OGRBackedVectorLayerTest, writesNativeGeometries){
    util::AutomaticTemporaryDirectory tmp {};
    // a computation produces fishnet value types, which have to be writable without the caller
    // converting them to an adapter by hand first
    auto native = VectorIO::nativeCopy(sourceLayer);
    EXPECT_TYPE<VectorLayer<Polygon<double>>>(native);

    Shapefile outputFile = Shapefile{tmp / std::filesystem::path("native.shp")};
    EXPECT_NO_FATAL_FAILURE(outputFile = VectorIO::write(native, outputFile));
    ASSERT_TRUE(std::filesystem::exists(outputFile.getPath()));

    auto readBack = VectorIO::read<OGRPolygonAdapter>(outputFile);
    ASSERT_EQ(native.size(), readBack.size());
    auto written = native.getGeometries();
    auto loaded = readBack.getGeometries();
    auto writtenIt = std::ranges::begin(written);
    auto loadedIt = std::ranges::begin(loaded);
    for(; writtenIt != std::ranges::end(written); ++writtenIt, ++loadedIt){
        EXPECT_NEAR((*writtenIt).area(), (*loadedIt).area(), 1e-9);
    }
}

TEST_F(OGRBackedVectorLayerTest, writesNativeGeometriesBuiltFromScratch){
    util::AutomaticTemporaryDirectory tmp {};
    // nothing here ever came from a data source
    VectorLayer<Polygon<double>> layer {sourceLayer.getSpatialReference()};
    Ring<double> boundary {std::vector<Vec2DReal>{{0,0},{0,1},{1,1},{1,0}}};
    layer.addGeometry(Polygon<double>(boundary));

    Shapefile outputFile = Shapefile{tmp / std::filesystem::path("scratch.shp")};
    EXPECT_NO_FATAL_FAILURE(outputFile = VectorIO::write(layer, outputFile));

    auto readBack = VectorIO::read<OGRPolygonAdapter>(outputFile);
    ASSERT_EQ(1u, readBack.size());
    auto loaded = readBack.getGeometries();
    EXPECT_NEAR(1.0, (*std::ranges::begin(loaded)).area(), 1e-9);
}
