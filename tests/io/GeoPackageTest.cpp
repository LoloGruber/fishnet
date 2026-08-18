#include <gtest/gtest.h>
#include <fstream>
#include <fishnet/TestUtil.hpp>
#include <fishnet/GeoPackage.hpp>
#include <fishnet/VectorIO.hpp>
#include <fishnet/Shapefile.hpp>

using namespace fishnet;
using namespace testutil;
namespace fs = std::filesystem;

class GeoPackageTest : public ::testing::Test {
protected:
    fs::path tempFile;
    void SetUp() override {
        tempFile = fs::temp_directory_path() / "test.gpkg";
        std::ofstream(tempFile).close(); // Create an empty GeoPackage file
    }   

    void TearDown() override {
        if (fs::exists(tempFile)) {
            fs::remove(tempFile);
        }
    }
};

TEST_F(GeoPackageTest, init) {
    fs::path invalidFile = fs::temp_directory_path() / "invalid.txt";
    std::ofstream(invalidFile).close(); // Create an invalid file
    EXPECT_THROW(GeoPackage {invalidFile}, std::invalid_argument);
    fs::remove(invalidFile);
    EXPECT_NO_THROW(GeoPackage {tempFile});
}

TEST_F(GeoPackageTest, remove) {
    fishnet::GeoPackage geopackage(tempFile);
    EXPECT_EXISTS(tempFile);
    EXPECT_TRUE(geopackage.remove());
    EXPECT_NOT_EXISTS(tempFile);
}

TEST_F(GeoPackageTest, move) {
    fishnet::GeoPackage geopackage(tempFile);
    fs::path newPath = fs::temp_directory_path() / "moved_test.gpkg";
    geopackage.move(newPath);
    EXPECT_NOT_EXISTS(tempFile);
    EXPECT_EXISTS(newPath);
    // Test invalid path
    fs::path invalidPath = fs::temp_directory_path() / "invalid_move.txt";
    EXPECT_THROW(geopackage.move(invalidPath), std::invalid_argument);
    EXPECT_EXISTS(newPath); // Original file should still exist
    EXPECT_NOT_EXISTS(invalidPath); // Invalid path should not exist
    // Cleanup
    fs::remove(newPath);
}

TEST_F(GeoPackageTest, copy) {
    fishnet::GeoPackage geopackage(tempFile);
    fs::path copyPath = fs::temp_directory_path() / "copy_test.gpkg";
    fishnet::GeoPackage copiedGeoPackage = geopackage.copy(copyPath);
    EXPECT_EXISTS(tempFile); // Original file should still exist
    EXPECT_EXISTS(copyPath); // Copied file should exist
    EXPECT_EQ(copiedGeoPackage.getPath(), copyPath);
    // Test invalid path
    fs::path invalidPath = fs::temp_directory_path() / "invalid_copy.txt";
    EXPECT_THROW(geopackage.copy(invalidPath), std::invalid_argument);
    EXPECT_NOT_EXISTS(invalidPath); // Invalid path should not exist
    // Cleanup
    fs::remove(copyPath);
}

TEST_F(GeoPackageTest, toString) {
    fishnet::GeoPackage geopackage(tempFile);
    std::string expectedString = "GeoPackage: " + tempFile.string();
    EXPECT_EQ(geopackage.toString(), expectedString);
}

TEST_F(GeoPackageTest, exists) {
    fishnet::GeoPackage geopackage(tempFile);
    EXPECT_EXISTS(tempFile);
    EXPECT_TRUE(geopackage.exists());
    // Remove the file and check again
    fs::remove(tempFile);
    EXPECT_NOT_EXISTS(tempFile);
    EXPECT_FALSE(geopackage.exists());
}

TEST(GeoPackageIO, readNonExistentFile) {
    fs::path nonExistent = fs::temp_directory_path() / "nonexistent.gpkg";
    auto result = VectorIO::tryRead<geometry::MultiPolygon<geometry::Polygon<double>>>(GeoPackage(nonExistent));
    EXPECT_FALSE(result.has_value());
}

TEST(GeoPackageIO, readSampleFile) {
    fs::path sampleFile = fs::path(__FILE__).parent_path().parent_path().parent_path() / "data" / "samples" / "Corvara_IT_Africapolis.gpkg";
    if (not fs::exists(sampleFile)) {
        GTEST_SKIP() << "Sample GeoPackage file not found: " << sampleFile;
    }
    auto result = VectorIO::tryRead<geometry::MultiPolygon<geometry::Polygon<double>>>(GeoPackage(sampleFile));
    EXPECT_TRUE(result.has_value());
    auto layer = result.value();
    EXPECT_SIZE(layer.getFeatures(), 7);
}

TEST(GeoPackageIO, writeAndReadBack) {
    fs::path sampleFile = fs::path(__FILE__).parent_path().parent_path().parent_path() / "data" / "samples" / "Corvara_IT_Africapolis.gpkg";
    fs::path outputFile = fs::temp_directory_path() / "test_output.gpkg";
    
    if (not fs::exists(sampleFile)) {
        GTEST_SKIP() << "Sample GeoPackage file not found: " << sampleFile;
    }
    
    // Read original
    auto original = VectorIO::read<geometry::MultiPolygon<geometry::Polygon<double>>>(GeoPackage(sampleFile));
    EXPECT_SIZE(original.getFeatures(), 7);
    
    // Write to new file
    auto written = VectorIO::write(original, GeoPackage(outputFile));
    EXPECT_EXISTS(outputFile);
    
    // Read back
    auto readBack = VectorIO::read<geometry::MultiPolygon<geometry::Polygon<double>>>(written);
    EXPECT_SIZE(readBack.getFeatures(), 7);
    
    // Cleanup
    fs::remove(outputFile);
}

TEST(GeoPackageIO, genericReadAbstractVectorFile) {
    fs::path sampleFile = fs::path(__FILE__).parent_path().parent_path().parent_path() / "data" / "samples" / "Corvara_IT_Africapolis.gpkg";
    if (not fs::exists(sampleFile)) {
        GTEST_SKIP() << "Sample GeoPackage file not found: " << sampleFile;
    }
    
    GeoPackage gpkg(sampleFile);
    auto layer = VectorIO::read<geometry::MultiPolygon<geometry::Polygon<double>>>(static_cast<const AbstractVectorFile&>(gpkg));
    EXPECT_SIZE(layer.getFeatures(), 7);
}

TEST(GeoPackageIO, genericTryReadAbstractVectorFile) {
    fs::path sampleFile = fs::path(__FILE__).parent_path().parent_path().parent_path() / "data" / "samples" / "Corvara_IT_Africapolis.gpkg";
    if (not fs::exists(sampleFile)) {
        GTEST_SKIP() << "Sample GeoPackage file not found: " << sampleFile;
    }
    
    GeoPackage gpkg(sampleFile);
    auto result = VectorIO::tryRead<geometry::MultiPolygon<geometry::Polygon<double>>>(static_cast<const AbstractVectorFile&>(gpkg));
    EXPECT_TRUE(result.has_value());
    EXPECT_SIZE(result.value().getFeatures(), 7);
}

TEST(GeoPackageIO, genericWriteAbstractVectorFile) {
    fs::path sampleFile = fs::path(__FILE__).parent_path().parent_path().parent_path() / "data" / "samples" / "Corvara_IT_Africapolis.gpkg";
    fs::path outputFile = fs::temp_directory_path() / "test_generic_output.gpkg";
    
    if (not fs::exists(sampleFile)) {
        GTEST_SKIP() << "Sample GeoPackage file not found: " << sampleFile;
    }
    
    auto original = VectorIO::read<geometry::MultiPolygon<geometry::Polygon<double>>>(GeoPackage(sampleFile));
    
    GeoPackage outputGpkg(outputFile);
    auto result = VectorIO::write(original, static_cast<const AbstractVectorFile&>(outputGpkg));
    EXPECT_EXISTS(outputFile);
    
    // Verify it was written correctly
    auto readBack = VectorIO::read<geometry::MultiPolygon<geometry::Polygon<double>>>(GeoPackage(outputFile));
    EXPECT_SIZE(readBack.getFeatures(), 7);
    
    // Cleanup
    fs::remove(outputFile);
}
