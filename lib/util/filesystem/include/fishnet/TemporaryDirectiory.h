#pragma once
#include <filesystem>

namespace fishnet::util{
/**
 * @brief Creates an empty temporary directory, which is clean up on destruction (-> RAII)
 * 
 */
class TemporaryDirectory{
private:
    std::filesystem::path directory;
public:
    TemporaryDirectory(const std::filesystem::path & directory);
    std::filesystem::path getDirectory() const noexcept;
    ~TemporaryDirectory();
};
}