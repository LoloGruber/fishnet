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
        auto fileType = getGISFileType(path);
        if (not fileType)
            return std::unexpected("Unsupported File Type");
        switch (fileType.value()) {
            case GISFileType::SHAPEFILE:
                return Shapefile(path);
            case GISFileType::GEOTIFF:
                if(std::filesystem::exists(path))
                    return GISConverter::convert(GeoTiff(path));
                else return Shapefile(path.parent_path() /path.stem().replace_extension(".shp"));
            default:
                break;
        }
        return std::unexpected("Unknown Error");
    }

    /**
     * @brief Get all GIS files in a directory/ or a single file from a path
     * 
     * @param directory search directory
     * @return std::vector<std::filesystem::path> list of gis files in that directory
     */
    static std::vector<std::filesystem::path> getGISFiles(const std::filesystem::path & path){
        std::vector<std::filesystem::path> gisFiles;
        auto dir = path;
        if(std::filesystem::is_symlink(path))
            dir = std::filesystem::read_symlink(path);
        if(not std::filesystem::is_directory(dir) && std::filesystem::is_regular_file(path) && getGISFileType(path).has_value()){
            gisFiles.emplace_back(path);
            return gisFiles;
        }
        for(auto && file: std::filesystem::directory_iterator(dir)){
            if(file.is_regular_file() && getGISFileType(file).has_value()){
                gisFiles.push_back(std::move(file));
            }
        }
        return gisFiles;
    }
};
}
