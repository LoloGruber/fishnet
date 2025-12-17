#include <fishnet/Shapefile.hpp>
#include <sstream>
#include <regex>
namespace fs= std::filesystem;
using namespace fishnet;

std::vector<fs::path> Shapefile::getAssociatedFiles() const{
    auto directory = this->getPath().parent_path();
    auto filename = this->getPath().stem();
    std::vector<fs::path> associates;
    for(const auto & file: fs::directory_iterator(directory)) {
        if(fs::is_regular_file(file)  and file.path().stem()==filename and supportsExtension(file.path()) ) {
            associates.push_back(file);
        }
    }
    return associates;
}

bool Shapefile::remove() const noexcept {
    if (not exists()) return false;
    try{
        for(const auto & file: getAssociatedFiles()){
            std::filesystem::remove(file);
        }
        return true;
    }catch(std::filesystem::filesystem_error & error){
        return false;
    }
}

Shapefile & Shapefile::move(const fs::path & path) {
    if(not supportsExtension(path))
        throw std::invalid_argument("Target path has to contain a valid shapefile extension");
    this->pathToFile = path;
    for(const auto & file : getAssociatedFiles()){
        fs::rename(file,fs::path(path.root_path() / fs::path(path.stem().string()+file.extension().string())));
    }
    return *this;
}

Shapefile & Shapefile::move(const fs::path & rootPath, std::string filename)  {
    return move(rootPath / fs::path(filename+".shp"));
}

Shapefile Shapefile::copy(const fs::path & path) const {
    //TODO ensure file exists
    if(not supportsExtension(path))
        throw std::invalid_argument("Target path has to contain a valid shapefile extension");
    for(const auto & file : getAssociatedFiles()){
        auto target = fs::path(path.parent_path() / fs::path(path.stem().string() + file.extension().string()));
        fs::copy(file,target);
    }
    return Shapefile {path};
}

Shapefile Shapefile::copy(const fs::path & rootPath, std::string filename) const {
    if(filename.ends_with(".shp"))
        return copy(rootPath / fs::path(filename));
    return copy(rootPath / fs::path(filename+".shp"));
}

std::string Shapefile::toString() const noexcept {
    std::ostringstream oss;
    oss << "Shapefile: ";
    oss << this->pathToFile;
    oss << std::endl;
    for(const auto & file: getAssociatedFiles()){
        oss << "\t" << file.string() << std::endl;
    }
    return oss.str();
}