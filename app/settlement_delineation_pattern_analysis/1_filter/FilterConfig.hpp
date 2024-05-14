#pragma once
#include <expected>
#include <fishnet/FunctionalConcepts.hpp>
#include <nlohmann/json.hpp>
#include "magic_enum.hpp"
#include "Filter.hpp"
#include "ApproxAreaFilter.hpp"
#include "ProjectedAreaFilter.hpp"
#include "InsidePolygonFilter.hpp"
#include "TaskConfig.hpp"

using json = nlohmann::json;

template<typename GeometryType>
static std::optional<fishnet::util::Predicate_t<GeometryType>> fromUnaryType(UnaryFilterType type,json const & filterDesc) {
    switch(type){
        case UnaryFilterType::ApproxAreaFilter:
            return ApproxAreaFilter::fromJson(filterDesc);
        case UnaryFilterType::ProjectedAreaFilter:
            return ProjectedAreaFilter::fromJson(filterDesc);
    }
    return std::nullopt;
}
template<typename GeometryType>
static std::optional<fishnet::util::BiPredicate_t<GeometryType>> fromBinaryType(BinaryFilterType type, json const & filterDesc){
    switch(type){
        case BinaryFilterType::InsidePolygonFilter:
            return InsidePolygonFilter();
    }
    return std::nullopt;
}

template<typename T>
struct FilterConfig:public TaskConfig{
    constexpr static const char * UNARY_FILTERS = "unary-filters";
    constexpr static const char * BINARY_FILTERS = "binary-filters";
    std::vector<fishnet::util::Predicate_t<T>> unaryPredicates;
    std::vector<fishnet::util::BiPredicate_t<T>> binaryPredicates;

    FilterConfig() = default;

    FilterConfig(const json & config):TaskConfig(config){
        for(const auto & jsonFilter: jsonDescription.at(UNARY_FILTERS)){
            std::string filterName;
            jsonFilter.at("name").get_to(filterName);
            auto unaryFilter = magic_enum::enum_cast<UnaryFilterType>(filterName)
                .and_then([&jsonFilter](UnaryFilterType unaryFilterType){return fromUnaryType<T>(unaryFilterType,jsonFilter);});
            if(not unaryFilter)
                throw std::runtime_error("Could not parse json to UnaryFilter:\n"+jsonFilter.dump()+"\nFilter name might not \""+filterName+"\" be supported");
            this->unaryPredicates.push_back(std::move(unaryFilter.value()));
        }
        for(const auto & jsonFilter : jsonDescription.at(BINARY_FILTERS)) {
            std::string filterName;
            jsonFilter.at("name").get_to(filterName);
            auto binaryFilter = magic_enum::enum_cast<BinaryFilterType>(filterName)
                .and_then([&jsonFilter](BinaryFilterType binaryFilterType){return fromBinaryType<T>(binaryFilterType,jsonFilter);});
            if(not binaryFilter) {
                throw std::runtime_error("Could not parse json to BinaryFilter:\n"+jsonFilter.dump()+"\nFilter name might not \""+filterName+"\" be supported");
            }
            this->binaryPredicates.push_back(std::move(binaryFilter.value()));
        }     
    }
};
