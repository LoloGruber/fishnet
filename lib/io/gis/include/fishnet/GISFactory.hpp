#pragma once
#include <expected>
#include <optional>

#include "Shapefile.hpp"
#include "GISFile.hpp"
#include "GeoTiff.hpp"
#include "GISConverter.hpp"

namespace fishnet {

class GISFactory {
public:
    /**
     * @brief Tries to construct ShapeFile handle from path.
     * If path does not point to .shp file, the function tries to convert it to a shapefile
     * @param path path to the file
     * @return std::expected<Shapefile,std::string>: Containing a shapefile on success or the error as a string
     */
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
        const auto & ext = path.extension().string();
        if(ext == ".shp" )
            return GISFileType::SHP;
        else if (ext  == ".tif" || ext  == ".tiff") {
            return GISFileType::TIFF;
        }
        return std::nullopt;
    }
};
}
