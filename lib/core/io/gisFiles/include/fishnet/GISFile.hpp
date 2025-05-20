#pragma once
#include <filesystem>
#include <optional>
#include <utility>
#include <fishnet/Printable.hpp>

namespace fishnet{

enum class GISFileType{
    SHAPEFILE,GEOTIFF
};

static std::optional<GISFileType> getGISFileType(const std::filesystem::path & path) {
    const auto & ext = path.extension().string();
    if(ext == ".shp" )
        return GISFileType::SHAPEFILE;
    else if (ext  == ".tif" || ext  == ".tiff") {
        return GISFileType::GEOTIFF;
    }
    return std::nullopt;
}

/**
 * Abstract class representing a GISFile, can only be inherited by predefined classes, namely AbstractVectorFile and AbstractRasterFile.
 */
class AbstractGISFile {
protected:
    std::filesystem::path pathToFile;

    void supportsOrThrow(const std::filesystem::path & path) const {
        if(not supports(path)){
            throw std::invalid_argument("File type not supported: "+ path.string());
        }
    }

private:
    explicit AbstractGISFile(std::filesystem::path pathToFile):pathToFile(std::move(pathToFile)){}

    explicit AbstractGISFile(const std::string& pathName):pathToFile(pathName){}

    friend class AbstractVectorFile;
    friend class AbstractRasterFile;
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

    [[nodiscard]] bool supports(const std::filesystem::path & path) const noexcept{
        auto type = getGISFileType(path);
        return type && type.value() == this->type();
    }

    constexpr virtual GISFileType type() const noexcept = 0;

    virtual bool remove() const noexcept = 0;

    virtual ~AbstractGISFile() = default;
};

class AbstractVectorFile: public AbstractGISFile {
public:
    explicit AbstractVectorFile(const std::filesystem::path & pathToFile): AbstractGISFile(pathToFile){}
    explicit AbstractVectorFile(const std::string& pathName): AbstractGISFile(pathName){}
    virtual ~AbstractVectorFile() = default;
};


class AbstractRasterFile: public AbstractGISFile {
public:
    explicit AbstractRasterFile(const std::filesystem::path & pathToFile): AbstractGISFile(pathToFile){}
    explicit AbstractRasterFile(const std::string& pathName): AbstractGISFile(pathName){}
    virtual ~AbstractRasterFile() = default;
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
};

/**
 * @brief Interface for all Vector GISFile types.
 * 
 * @tparam File Vector file implementation type
 */
template<typename File>
concept VectorGISFile = GISFile<File> && std::derived_from<File,AbstractVectorFile>;

/**
 * @brief Interface for all Raster GISFile types.
 * 
 * @tparam File Raster file implementation type
 */
template<typename File>
concept RasterGISFile = GISFile<File> && std::derived_from<File,AbstractRasterFile>;
}