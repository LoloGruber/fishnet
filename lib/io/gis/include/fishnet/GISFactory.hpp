//
// Created by lolo on 18.04.24.
//
#pragma once
#include "Shapefile.hpp"
#include "GISFile.hpp"
#include "GeoTiff.hpp"
#include "GISConverter.hpp"

#include <expected>
#include <optional>

namespace fishnet {

class GISFactory {
public:
    static std::expected<Shapefile,std::string> asShapefile(const std::filesystem::path& path){
        auto fileType = getType(path);
        if (not fileType)
            return std::unexpected("Unsupported File Type");
        switch (fileType.value()) {
            case GISFileType::SHP:
                return Shapefile(path);
            case GISFileType::TIFF:
                if(std::filesystem::exists(path))
                    return GISConverter::convert(GeoTiff(path));
                else return Shapefile(path.parent_path() /path.stem().replace_extension(".shp"));
        }
        return std::unexpected("Unknown Error");

    }

    static std::optional<GISFileType> getType(const std::filesystem::path & path) {
        if(Shapefile::supportsExtension(path))
            return GISFileType::SHP;
        else if (path.extension().string() == ".tiff") {
            return GISFileType::TIFF;
        }
        return std::nullopt;
    }
};
}
