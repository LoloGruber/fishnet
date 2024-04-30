#pragma once
#include <filesystem>
#include <vector>
#include <ranges>
#include <algorithm>
#include <string_view>
#include <ostream>
#include "GISFile.hpp"
namespace fishnet{

class Shapefile: public AbstractGISFile{
private:

    [[nodiscard]] std::vector<std::filesystem::path> getAssociatedFiles()const;

    constexpr static inline std::array<std::string,5> FILE_EXTENSIONS = {
        ".shp",".shx",".prj",".dbf",".cpg"
    };

    constexpr static inline std::array<std::string,4> REQUIRED_FILES = {
        ".shp",".shx",".dbf",".prj"
    };

public:
    static bool supportsExtension(const std::filesystem::path & path){
        return std::ranges::any_of(FILE_EXTENSIONS,[&path](const auto & ext){return ext == path.extension().string();});
    }

    static bool isValid(const std::filesystem::path & path) {
        return std::ranges::all_of(REQUIRED_FILES,[&path](const auto & ext){return std::filesystem::exists(path.parent_path() / std::filesystem::path(path.stem().string()+ext));});
    }

    Shapefile(const std::filesystem::path & path):AbstractGISFile(path){
        if(not supportsExtension(path))
            throw std::invalid_argument("Not a shapefile!");
    };

    [[nodiscard]] bool exists() const noexcept{
        return AbstractGISFile::exists() && isValid(this->pathToFile);
    }

    operator bool() const noexcept {
        return this->exists();
    }

    [[nodiscard]] Shapefile changeFilename(const std::string & filename) const noexcept;

    [[nodiscard]] Shapefile appendToFilename(const std::string & postfix) const noexcept;

    [[nodiscard]] Shapefile incrementFileVersion() const noexcept;

    bool remove() const noexcept;

    [[nodiscard]]constexpr static GISFileType type()  noexcept{
        return GISFileType::SHP;
    }

    Shapefile & move(const std::filesystem::path & path);

    Shapefile & move(const std::filesystem::path & rootPath, std::string filename) ;

    Shapefile copy(const std::filesystem::path & path) const;

    Shapefile copy(const std::filesystem::path & rootPath, std::string filename) const;

    std::string toString() const noexcept;

};
static_assert(GISFile<Shapefile>);
}

