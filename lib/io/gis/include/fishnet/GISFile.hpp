#pragma once
#include <filesystem>
#include <utility>
#include <fishnet/Printable.hpp>

namespace fishnet{

enum class GISFileType {
    SHP,TIFF
};
/**
 * Abstract class representing a GISFile (namely Shapefile or GeoTIFF)
 */
class AbstractGISFile {
protected:
    std::filesystem::path pathToFile;

    explicit AbstractGISFile(std::filesystem::path pathToFile):pathToFile(std::move(pathToFile)){}

    explicit AbstractGISFile(const std::string& pathName):pathToFile(pathName){}
public:
    [[nodiscard]] const std::filesystem::path & getPath() const noexcept {
        return pathToFile;
    }

    operator bool ()const noexcept {
        return this->exists();
    }

    [[nodiscard]] bool exists() const noexcept {
        return std::filesystem::exists(this->pathToFile);
    }

    virtual ~AbstractGISFile() = default;
};

/**
 * @brief Interface for GISFile types.
 * Implementation have to inherit from AbstractGISFile and have to printable to the console.
 * @tparam File GIS file implementation type
 */
template<typename File>
concept GISFile = std::derived_from<File,AbstractGISFile> && util::Printable<File> && requires (const File & constF, File & file, std::filesystem::path p){
    {file.move(p)}-> std::same_as<File &>;
    {constF.copy(p)} -> std::same_as<File>;
    {File(p)};
    {File::type()} -> std::same_as<GISFileType>;
};
}