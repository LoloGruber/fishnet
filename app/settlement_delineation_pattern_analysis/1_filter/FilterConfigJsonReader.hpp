#pragma once
#include "SettlementFilterTask.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <expected>
#include "Filter.hpp"
#include "ApproxAreaFilter.hpp"
#include "ProjectedAreaFilter.hpp"
#include "InsidePolygonFilter.hpp"

using json = nlohmann::json;

class FilterConfigJsonReader{
private:
    json config;
    FilterConfigJsonReader(json jsonConfig):config(std::move(jsonConfig)){}
public:
    constexpr static const char * UNARY_FILTERS = "unary-filters";
    constexpr static const char * BINARY_FILTERS = "binary-filters";

    template<typename T>
    static std::expected<util::Predicate_t<T>,std::string> fromUnaryType(UnaryFilterType type,json const & filterDesc) {
        switch(type){
            case UnaryFilterType::ApproxAreaFilter:
                 return ApproxAreaFilter::fromJson(filterDesc);
            case UnaryFilterType::ProjectedAreaFilter:
                return ProjectedAreaFilter::fromJson(filterDesc);
        }
        return std::unexpected("Unexpected UnaryFilterType");
    }

    template<typename T>
    static std::expected<util::BiPredicate_t<T>,std::string> fromBinaryType(BinaryFilterType type, json const & filterDesc){
        switch(type){
            case BinaryFilterType::InsidePolygonFilter:
                return InsidePolygonFilter();
        }
        return std::unexpected("Unexpected BinaryFilterType");
    }

    template<typename T>
    [[nodiscard]] std::expected<void,std::string> read(SettlementFilterTask<T> & task) const noexcept {
        try{
            for(const auto & jsonFilter:  config.at(UNARY_FILTERS)){
                std::string filterName;
                jsonFilter.at("name").get_to(filterName);
                auto filterType = FILTERS::UNARY.get(filterName);
                if(not filterType) {
                    return std::unexpected("Filter name \""+filterName+"\" not supported");
                }
                auto predicate = fromUnaryType<T>(filterType.value(),jsonFilter);
                if (not predicate){
                    return std::unexpected(predicate.error());
                }
                task.addPredicate(predicate.value());
            }
            for(const auto & jsonFilter : config.at(BINARY_FILTERS)) {
                std::string filterName;
                jsonFilter.at("name").get_to(filterName);
                auto filterType = FILTERS::BINARY.get(filterName);
                if(not filterType) {
                    return std::unexpected("Filter name \""+filterName+"\" not supported");
                }
                auto biPredicate = fromBinaryType<T>(filterType.value(),jsonFilter);
                if(not biPredicate)
                    return std::unexpected(biPredicate.error());
                task.addBiPredicate(biPredicate.value());
            }
            return {};
        }catch(const json::exception & e){
            return std::unexpected(std::string("Could not parse attributes of filter config\n")+e.what());
        }
    }

    static std::expected<FilterConfigJsonReader,std::string> createFromString(const std::string & jsonString) noexcept {
        try{
            return FilterConfigJsonReader(json::parse(jsonString));
        }catch(const json::parse_error & e) {
            return std::unexpected("Invalid json string:\n\""+jsonString+"\"");
        }
    }

    static std::expected<FilterConfigJsonReader,std::string> createFromPath(const std::filesystem::path & pathToConfig) noexcept{
        try{
            return FilterConfigJsonReader(json::parse(std::ifstream(pathToConfig)));
        }catch(const json::parse_error & e){
            return std::unexpected("Coult not read json from file:\n\""+pathToConfig.string()+"\"");
        }
    }

};
