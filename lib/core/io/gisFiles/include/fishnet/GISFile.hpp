#pragma once
#include <filesystem>
#include <optional>
#include <utility>
#include <fishnet/Printable.hpp>
#include <fishnet/PathHelper.h>

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

    /**
     * @brief Tests if the GISFile exists on the filesystem.
     * 
     * @return true 
     * @return false 
     */
    [[nodiscard]] bool exists() const noexcept {
        return std::filesystem::exists(this->pathToFile);
    }

    /**
     * @brief Checks if the supplied path is supported by the concrete GISFile implementation.
     * 
     * @param path 
     * @return true 
     * @return false 
     */
    [[nodiscard]] bool supports(const std::filesystem::path & path) const noexcept{
        auto type = getGISFileType(path);
        return type && type.value() == this->type();
    }

    /**
     * @brief Modifies the path of the GISFile by changing the filename (extension is preserved).
     * 
     * @param filename new file stem of the GISFile
     */
    void changeFilename(const std::string & filename) noexcept {
        this->pathToFile = util::PathHelper::changeFilename(this->pathToFile, filename);
    }

    /**
     * @brief Appends a suffix to the filename / stem of the GISFile (extension is preserved).
     * 
     * @param suffix suffix to append to the filename / stem
     */
    void appendToFilename(const std::string & suffix) noexcept {
        changeFilename(this->pathToFile.stem().string() + suffix);
    }

    /**
     * @brief Returns the type of GISFile
     * 
     * @return constexpr GISFileType 
     */
    constexpr virtual GISFileType type() const noexcept = 0;

    /**
     * @brief Removes the file from the filesystem
     * 
     * @return true success
     * @return false error
     */
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