#include <fishnet/GeoPackage.hpp>
#include <filesystem>

using namespace fishnet;
namespace fs = std::filesystem;

bool GeoPackage::remove() const noexcept {
    return fs::remove(this->pathToFile);
}

GeoPackage & GeoPackage::move(std::filesystem::path const & path) {
    supportsOrThrow(path);
    fs::rename(this->pathToFile, path);
    this->pathToFile = path;
    return *this;
}

GeoPackage GeoPackage::copy(std::filesystem::path const & path) const {
    supportsOrThrow(path);
    fs::copy(this->pathToFile, path);
    return GeoPackage(path);
}

std::string GeoPackage::toString() const noexcept {
    return "GeoPackage: "+ this->pathToFile.string();
}   