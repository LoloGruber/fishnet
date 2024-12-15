#pragma once
#include <filesystem>
#include <regex>
#include <optional>
#include <fishnet/Vec2D.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <nlohmann/json.hpp>

enum class NeighbouringFilesPredicateType {
    TILES, WSF
};

struct NeighbouringFileTilesPredicate{
    
    std::optional<fishnet::geometry::Vec2D<int>> tileCoordinateFromFilenames(const std::string & input)const noexcept{
        std::regex pattern(R"((.+)_(-?\d+)_(-?\d+)\.shp)");
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

struct NeighbouringWSFFilesPredicate{

    std::optional<fishnet::geometry::Vec2D<int>> spatialCoordinatesFromFilename(const std::string & input)const noexcept{
        std::regex pattern(R"((.+)_(-?\d+)_(-?\d+)\..+)");
        std::smatch matches;

        if (std::regex_match(input, matches, pattern) && matches.size() == 4) {
            int x = std::stoi(matches[2].str());
            int y = std::stoi(matches[3].str());
            return fishnet::geometry::Vec2D(x,y);
        }
        return std::nullopt;
    }


    bool operator()(std::filesystem::path const & lhs, std::filesystem::path const & rhs) const noexcept {
        auto l = spatialCoordinatesFromFilename(lhs.filename().string());
        auto r = spatialCoordinatesFromFilename(rhs.filename().string());
        if(l and r){
            auto leftCoordinate = l.value();
            auto rightCoordinate = r.value();
            return abs(leftCoordinate.x-rightCoordinate.x) <= 2 && abs(leftCoordinate.y-rightCoordinate.y) <= 2;
        }   
        return false;
    }
};

static std::optional<fishnet::util::BiPredicate_t<std::filesystem::path>> fromJson(NeighbouringFilesPredicateType type,const nlohmann::json & desc ){
    switch(type){
        case NeighbouringFilesPredicateType::TILES:
            return NeighbouringFileTilesPredicate();
        case NeighbouringFilesPredicateType::WSF:
            return NeighbouringWSFFilesPredicate();
        default:
            return std::nullopt;
    }
}