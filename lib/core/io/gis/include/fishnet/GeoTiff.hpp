#pragma once
#include "GISFile.hpp"
namespace fishnet {
/**
 * @brief GeoTiff handle implementation
 * 
 */
class GeoTiff:public AbstractGISFile{
public:
    GeoTiff(std::filesystem::path path):AbstractGISFile(path){}

    bool remove() const;

    constexpr static GISFileType type()  noexcept{
        return GISFileType::TIFF;
    }

    GeoTiff & move(std::filesystem::path const & path);

    GeoTiff copy(std::filesystem::path const & path) const;

    std::string toString() const noexcept;
};

static_assert(GISFile<GeoTiff>);
}