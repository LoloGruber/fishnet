#pragma once
#include "GISFile.hpp"
namespace fishnet {
/**
 * @brief GeoTiff handle implementation
 * 
 */
class GeoTiff:public AbstractRasterFile{
public:
    GeoTiff(std::filesystem::path path):AbstractRasterFile(path){
        if(not supports(path)){
            throw std::invalid_argument("Not a GeoTiff file: "+ path.string());
        }
    }

    bool remove() const noexcept override;

    constexpr GISFileType type() const noexcept override {
        return GISFileType::GEOTIFF;
    }

    GeoTiff & move(std::filesystem::path const & path);

    GeoTiff copy(std::filesystem::path const & path) const;

    std::string toString() const noexcept;
};

static_assert(GISFile<GeoTiff>);
static_assert(!VectorGISFile<GeoTiff>);
static_assert(RasterGISFile<GeoTiff>);
}