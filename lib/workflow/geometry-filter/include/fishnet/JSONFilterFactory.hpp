#pragma once
#include <nlohmann/json.hpp>
#include <magic_enum.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/Option.hpp>
#include "FilterType.hpp"
#include "ApproxAreaFilter.hpp"
#include "InsidePolygonFilter.hpp"

template<fishnet::geometry::GeometryObject G>
class JSONFilterFactory{
    static_assert(fishnet::util::Predicate<ApproxAreaFilter, G>);
public:
    static fishnet::Option<fishnet::util::Predicate_t<G>> getFilter(const nlohmann::json & filterDesc) {
        return magic_enum::enum_cast<FilterType>(filterDesc.at("type").get<std::string>()).and_then([&filterDesc](FilterType type)->std::optional<fishnet::util::Predicate_t<G>>{
            switch(type){
                case FilterType::ProjectedAreaFilter:
                    break;
                case FilterType::ApproxAreaFilter:
                    return ApproxAreaFilter(filterDesc.at("required-area").get<double>());
            }
            return std::nullopt;
        });
    }

    static fishnet::Option<fishnet::util::BiPredicate_t<G,G>> getBinaryFilter(const nlohmann::json & filterDesc){
        return magic_enum::enum_cast<BinaryFilterType>(filterDesc.at("type").get<std::string>()).and_then([](BinaryFilterType type)->std::optional<fishnet::util::BiPredicate_t<G,G>>{
            switch(type){
                case BinaryFilterType::InsidePolygonFilter:
                    return InsidePolygonFilter();
            }
            return std::nullopt;
        });
    }

    static std::vector<fishnet::util::Predicate_t<G>> getFilters(const nlohmann::json & filtersDesc){
        std::vector<fishnet::util::Predicate_t<G>> filters;
        for(const auto & filterDesc: filtersDesc){
            auto filter = getFilter(filterDesc);
            if(filter.has_value()){
                filters.push_back(std::move(filter.value()));
            }
        }
        return filters;
    }

    static std::vector<fishnet::util::BiPredicate_t<G,G>> getBinaryFilters(const nlohmann::json & filtersDesc){
        std::vector<fishnet::util::BiPredicate_t<G,G>> filters;
        for(const auto & filterDesc: filtersDesc){
            auto filter = getBinaryFilter(filterDesc);
            if(filter.has_value()){
                filters.push_back(std::move(filter.value()));
            }
        }
        return filters;
    }
};

