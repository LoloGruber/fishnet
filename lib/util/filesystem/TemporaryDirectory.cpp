#include "TemporaryDirectiory.hpp"
#include <stdexcept>

namespace util{
TemporaryDirectory::TemporaryDirectory(const std::filesystem::path & directory):directory(directory){
        if (std::filesystem::exists(directory) && not std::filesystem::is_empty(directory)){
            throw std::invalid_argument("Directory already exists and is not empty");
        }
        std::filesystem::create_directory(directory);
}

std::filesystem::path TemporaryDirectory::getDirectory()const noexcept{
    return this->directory;
}

TemporaryDirectory::~TemporaryDirectory(){
    std::filesystem::remove_all(directory);
    std::filesystem::remove(directory);
}

}