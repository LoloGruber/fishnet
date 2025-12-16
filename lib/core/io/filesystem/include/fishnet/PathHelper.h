#pragma once
#include <filesystem>
#include <string_view>

namespace fishnet::util {

constexpr static const char * PROJECT_NAME = "fishnet";
/**
 * @brief Static Helper class to get the current Path, find the projectDirectory, obtain an absolute canonical path and replace filenames
 * 
 */
class PathHelper{
public:
    static std::filesystem::path getCurrentPath() noexcept;
    static std::filesystem::path projectDirectory(std::string_view searchPattern) noexcept;
    static std::filesystem::path projectDirectory() noexcept;
    static std::filesystem::path appendToFilename(const std::filesystem::path & path,std::string_view suffix) noexcept;
    static std::filesystem::path changeFilename(const std::filesystem::path & path, std::string_view filename) noexcept;
    static std::filesystem::path absoluteCanonical(const std::filesystem::path & path) noexcept;
};
}