#include <fishnet/PathHelper.h>

namespace fishnet::util {

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

std::filesystem::path PathHelper::appendToFilename(const std::filesystem::path & path,std::string_view suffix) noexcept {
    return changeFilename(path,path.stem().string()+std::string(suffix));
}
std::filesystem::path PathHelper::changeFilename(const std::filesystem::path & path, std::string_view filename) noexcept{
    auto ext = path.extension();
    auto result = path.parent_path() / std::filesystem::path(filename);
    if (not result.has_extension())
        return result.replace_extension(ext);
    return result;
}
}