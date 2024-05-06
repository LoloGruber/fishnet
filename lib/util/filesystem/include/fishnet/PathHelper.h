#pragma once
#include <filesystem>
#include <string_view>

namespace fishnet::util {

constexpr static const char * PROJECT_NAME = "fishnet";
class PathHelper{
public:
    static std::filesystem::path getCurrentPath() noexcept;
    static std::filesystem::path projectDirectory(std::string_view searchPattern) noexcept;
    static std::filesystem::path projectDirectory() noexcept;
};
}