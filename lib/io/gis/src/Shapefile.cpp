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

Shapefile Shapefile::changeFilename(const std::string & filename) const noexcept {
    if (not filename.ends_with(".shp"))
        return this->pathToFile.parent_path() / std::filesystem::path(filename+".shp");
    return this->pathToFile.parent_path() / std::filesystem::path(filename);
}

Shapefile Shapefile::appendToFilename(const std::string & postfix) const noexcept {
    return changeFilename(this->pathToFile.stem().string() + postfix);
}

Shapefile Shapefile::incrementFileVersion() const noexcept {
    std::regex endsWithVersionRegex {"_(\\d+)$"};
    const std::string & currentFilename = this->pathToFile.stem().string();
    std::smatch matcher;
    if(std::regex_search(currentFilename,matcher,endsWithVersionRegex)){
        size_t splitIndex = currentFilename.find_last_of('_');
        std::string filename = currentFilename.substr(0,splitIndex);
        int version = std::stoi(matcher[1].str()) + 1;
        return this->changeFilename(filename+"_"+std::to_string(version));
    }
    return this->appendToFilename("_1");
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
    Shapefile destination {path};
    for(const auto & file : getAssociatedFiles()){
        auto target = fs::path(path.parent_path() / fs::path(path.stem().string() + file.extension().string()));
        fs::copy(file,target);
    }
    return destination;
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