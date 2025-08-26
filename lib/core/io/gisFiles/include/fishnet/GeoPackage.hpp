#pragma once
#include "GISFile.hpp"

namespace fishnet {
/**
 * @brief GeoPackage handle
 * 
 */
class GeoPackage: public AbstractGISFile {
public:
    GeoPackage(std::filesystem::path path):AbstractGISFile(path){
        if(not supports(path)){
            throw std::invalid_argument("Not a GeoPackage file: "+ path.string());
        }
    }

    bool remove() const noexcept override;

    constexpr GISFileType type() const noexcept override {
        return GISFileType::GEOPACKAGE;
    }

    GeoPackage & move(std::filesystem::path const & path);

    GeoPackage copy(std::filesystem::path const & path) const;

    std::string toString() const noexcept;
};
static_assert(GISFile<GeoPackage>);
}