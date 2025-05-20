#include <gtest/gtest.h>
#include <fishnet/GeoTiff.hpp>
#include <fstream>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;
using namespace fishnet;

class GeoTiffTest : public ::testing::Test {
protected:
    fs::path tempFile;

    void SetUp() override {
        tempFile = fs::temp_directory_path() / "test.tif";
        std::ofstream(tempFile).close(); // Create an empty GeoTiff file
    }

    void TearDown() override {
        if (fs::exists(tempFile)) {
            fs::remove(tempFile);
        }
    }
};

TEST_F(GeoTiffTest, init) {
    fs::path invalidFile = fs::temp_directory_path() / "invalid.txt";
    std::ofstream(invalidFile).close(); // Create an invalid file
    EXPECT_THROW(GeoTiff {invalidFile}, std::invalid_argument);
    fs::remove(invalidFile);
}

TEST_F(GeoTiffTest, remove) {
    fishnet::GeoTiff geotiff(tempFile);
    EXPECT_TRUE(fs::exists(tempFile));
    EXPECT_TRUE(geotiff.remove());
    EXPECT_FALSE(fs::exists(tempFile));
}

TEST_F(GeoTiffTest, move) {
    fishnet::GeoTiff geotiff(tempFile);
    fs::path newPath = fs::temp_directory_path() / "moved_test.tiff";
    geotiff.move(newPath);
    EXPECT_FALSE(fs::exists(tempFile));
    EXPECT_TRUE(fs::exists(newPath));
    // Test invalid path
    fs::path invalidPath = fs::temp_directory_path() / "invalid_move.txt";
    EXPECT_THROW(geotiff.move(invalidPath), std::invalid_argument);
    EXPECT_TRUE(fs::exists(newPath)); // Original file should still exist
    EXPECT_FALSE(fs::exists(invalidPath)); // Invalid path should not exist
    // Cleanup
    fs::remove(newPath);
}

TEST_F(GeoTiffTest, copy) {
    fishnet::GeoTiff geotiff(tempFile);
    fs::path copyPath = fs::temp_directory_path() / "copy_test.tif";
    fishnet::GeoTiff copiedGeoTiff = geotiff.copy(copyPath);
    EXPECT_TRUE(fs::exists(tempFile));
    EXPECT_TRUE(fs::exists(copyPath));
    fs::remove(copyPath);

    // Test invalid path
    fs::path invalidPath = fs::temp_directory_path() / "invalid_copy.txt";
    EXPECT_THROW(geotiff.copy(invalidPath), std::invalid_argument);
    EXPECT_TRUE(fs::exists(tempFile)); // Original file should still exist
    EXPECT_FALSE(fs::exists(invalidPath)); // Invalid path should not exist
}

TEST_F(GeoTiffTest, toString) {
    fishnet::GeoTiff geotiff(tempFile);
    EXPECT_EQ(geotiff.toString(), "Geotiff: " + tempFile.string());
}
TEST_F(GeoTiffTest, exists) {
    fishnet::GeoTiff geotiff(tempFile);
    EXPECT_TRUE(fs::exists(tempFile));
    EXPECT_TRUE(geotiff.exists());

    // Remove the file and check again
    fs::remove(tempFile);
    EXPECT_FALSE(fs::exists(tempFile));
    EXPECT_FALSE(geotiff.exists());
}

