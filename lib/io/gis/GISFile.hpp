#pragma once
#include <filesystem>

namespace fishnet{

template<typename File>
concept GISFile=requires(const File & constf,File & f, const std::filesystem::path & p){
    {File(p)};
    {constf.getPath()} -> std::convertible_to<std::filesystem::path>;
    {f.move(p)} -> std::convertible_to<File>;
    {constf.copy(p)}->std::convertible_to<File>;
};
}