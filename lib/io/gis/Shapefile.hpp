#pragma once
#include <filesystem>
#include <vector>
#include <ranges>
#include <algorithm>
#include <string_view>
#include <ostream>
#include "GISFile.hpp"
namespace fishnet{

class Shapefile{
private:
    std::filesystem::path pathToShp;

    std::vector<std::filesystem::path> getAssociatedFiles()const;

    const static inline std::array<std::string,5> FILE_EXTENSIONS = {
        ".shp",".shx",".prj",".dbf",".cpg"
    };

public:
    static bool supportsExtension(const std::filesystem::path & path){
        return std::ranges::any_of(FILE_EXTENSIONS,[&path](const auto & ext){return ext == path.extension().string();});
    }

    Shapefile(const std::filesystem::path & path):pathToShp(path){
        if(not supportsExtension(path))
            throw std::invalid_argument("Not a shapefile!");
    };

    const std::filesystem::path & getPath() const noexcept{
        return this->pathToShp;
    }

    Shapefile changeFilename(const std::string & filename) const noexcept;

    Shapefile appendToFilename(const std::string & postfix) const noexcept;

    Shapefile incrementFileVersion() const noexcept;

    bool exists() const noexcept;

    bool remove() const noexcept;

    Shapefile & move(const std::filesystem::path & path) ;

    Shapefile & move(const std::filesystem::path & rootPath, std::string filename) ;

    Shapefile copy(const std::filesystem::path & path) const;

    Shapefile copy(const std::filesystem::path & rootPath, std::string filename) const;

    std::string toString() const noexcept;

};
static_assert(GISFile<Shapefile>);
}

