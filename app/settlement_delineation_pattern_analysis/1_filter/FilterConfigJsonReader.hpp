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
#include "JsonConfigReader.hpp"

using json = nlohmann::json;

class FilterConfigJsonReader:public BaseJsonConfigReader{
public:
    constexpr static const char * UNARY_FILTERS = "unary-filters";
    constexpr static const char * BINARY_FILTERS = "binary-filters";

    FilterConfigJsonReader(const std::string & jsonString):BaseJsonConfigReader(jsonString) {}

    FilterConfigJsonReader(const std::filesystem::path & path):BaseJsonConfigReader(path) {}

    template<GeometryObject GeometryType>
    static std::expected<fishnet::util::Predicate_t<GeometryType>,std::string> fromUnaryType(UnaryFilterType type,json const & filterDesc) {
        switch(type){
            case UnaryFilterType::ApproxAreaFilter:
                 return ApproxAreaFilter::fromJson(filterDesc);
            case UnaryFilterType::ProjectedAreaFilter:
                return ProjectedAreaFilter::fromJson(filterDesc);
        }
        return std::unexpected("Unexpected UnaryFilterType");
    }
    template<GeometryObject GeometryType>
    static std::expected<fishnet::util::BiPredicate_t<GeometryType>,std::string> fromBinaryType(BinaryFilterType type, json const & filterDesc){
        switch(type){
            case BinaryFilterType::InsidePolygonFilter:
                return InsidePolygonFilter();
        }
        return std::unexpected("Unexpected BinaryFilterType");
    }

    template<GeometryObject GeometryType>
    void parse(SettlementFilterTask<GeometryType> & task) const noexcept {
        task.writeDescLine("\tUnary-Filters:");
        for(const auto & jsonFilter:  this->config.at(UNARY_FILTERS)){
            std::string filterName;
            jsonFilter.at("name").get_to(filterName);
            auto filterType = FILTERS::UNARY.get(filterName);
            if(not filterType) {
                std::cerr << "Filter name \""+filterName+"\" not supported" << std::endl;
            }
            auto predicate = fromUnaryType<GeometryType>(filterType.value(),jsonFilter);
            if (not predicate){
                std::cerr << predicate.error() << std::endl;
            }
            task.addPredicate(predicate.value());
            task.writeDescLine("\t\t"+jsonFilter.dump());
        }
        task.writeDescLine("\tBinary-Filters:");
        for(const auto & jsonFilter : this->config.at(BINARY_FILTERS)) {
            std::string filterName;
            jsonFilter.at("name").get_to(filterName);
            auto filterType = FILTERS::BINARY.get(filterName);
            if(not filterType) {
                std::cerr << "Filter name \""+filterName+"\" not supported" << std::endl;
            }
            auto biPredicate = fromBinaryType<GeometryType>(filterType.value(),jsonFilter);
            if(not biPredicate)
                std::cerr << biPredicate.error() << std::endl;
            task.addBiPredicate(biPredicate.value());
            task.writeDescLine("\t\t"+jsonFilter.dump());
        }
    }
};
static_assert(JsonConfigReader<FilterConfigJsonReader,SettlementFilterTask<fishnet::geometry::Polygon<double>>>);
