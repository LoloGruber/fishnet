#include <fishnet/GeoTiff.hpp>
#include <filesystem>

using namespace fishnet;
namespace fs = std::filesystem;

bool GeoTiff::remove() const noexcept {
    return fs::remove(this->pathToFile);
}

GeoTiff & GeoTiff::move(std::filesystem::path const & path) {
    supportsOrThrow(path);
    fs::rename(this->pathToFile, path);
    this->pathToFile = path;
    return *this;
}

GeoTiff GeoTiff::copy(std::filesystem::path const & path) const {
    supportsOrThrow(path);
    fs::copy(this->pathToFile, path);
    return GeoTiff(path);
}

std::string GeoTiff::toString() const noexcept {
    return "Geotiff: "+ this->pathToFile.string();
}