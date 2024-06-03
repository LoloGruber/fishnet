#pragma once
#include <filesystem>
#include <regex>
#include <optional>
#include <fishnet/Vec2D.hpp>

struct NeighbouringFileTilesPredicate{
    
    std::optional<fishnet::geometry::Vec2D<int>> tileCoordinateFromFilenames(const std::string & input)const noexcept{
        std::regex pattern(R"((.+)_(\d+)_(\d+)\.shp)");
        std::smatch matches;

        if (std::regex_match(input, matches, pattern) && matches.size() == 4) {
            int x = std::stoi(matches[2].str());
            int y = std::stoi(matches[3].str());
            return fishnet::geometry::Vec2D(x,y);
        }
        return std::nullopt;
    }

    bool operator()(std::filesystem::path const & lhs, std::filesystem::path const & rhs) const noexcept {
        auto l = tileCoordinateFromFilenames(lhs.filename().string());
        auto r = tileCoordinateFromFilenames(rhs.filename().string());
        if(l and r){
            return l->distance(r.value()) <= 1.5;
        }   
        return false;
    }
};