#pragma once
#include <filesystem>
namespace util{
class TemporaryDirectory{
private:
    std::filesystem::path directory;
public:
    TemporaryDirectory(const std::filesystem::path & directory);
    std::filesystem::path getDirectory() const noexcept;
    ~TemporaryDirectory();
};
}