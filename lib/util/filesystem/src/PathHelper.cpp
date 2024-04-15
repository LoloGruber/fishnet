#include <fishnet/PathHelper.h>

namespace util {

std::filesystem::path PathHelper::getCurrentPath() noexcept {
    return std::filesystem::current_path();
}

std::filesystem::path PathHelper::projectDirectory(std::string_view searchPattern) noexcept {
    std::filesystem::path projectDir = PathHelper::getCurrentPath();
    while(projectDir != projectDir.root_directory() and projectDir.filename() != searchPattern){
        projectDir = projectDir.parent_path();
    }
    return projectDir;
}

std::filesystem::path PathHelper::projectDirectory() noexcept {
    return projectDirectory(util::PROJECT_NAME);
}
}