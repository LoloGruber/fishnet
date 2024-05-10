//
// Created by lolo on 18.04.24.
//
#pragma once
#include <fishnet/FunctionalConcepts.hpp>
#include <string_view>
#include <expected>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fishnet/StaticMap.hpp>

enum class UnaryFilterType{
    ApproxAreaFilter, ProjectedAreaFilter
};

enum class BinaryFilterType{
    InsidePolygonFilter
};

namespace FILTERS{

// constexpr static fishnet::util::StaticMap<const char *,UnaryFilterType> UNARY = {{
//     {"ApproxAreaFilter",UnaryFilterType::ApproxAreaFilter},
//     {"ProjectedAreaFilter",UnaryFilterType::ProjectedAreaFilter}
// }};

using namespace std::literals::string_view_literals;
constexpr static std::array<std::pair<std::string_view,UnaryFilterType>,2> unary_data{{
    {"ApproxAreaFilter"sv,UnaryFilterType::ApproxAreaFilter},
    {"ProjectedAreaFilter"sv,UnaryFilterType::ProjectedAreaFilter}
}};

constexpr static std::array<std::pair<std::string_view,BinaryFilterType>,1> binary_data {{
    {"InsidePolygonFilter",BinaryFilterType::InsidePolygonFilter}
}};

constexpr static fishnet::util::StaticMap<std::string_view,UnaryFilterType,unary_data.size()> UNARY {unary_data};

constexpr static inline fishnet::util::StaticMap<std::string_view,BinaryFilterType,binary_data.size()> BINARY {binary_data};
}


template<typename F,typename T>
concept UnaryFilter = fishnet::util::Predicate<F,T> && requires(const nlohmann::json & json) {
    {F::type()} -> std::same_as<UnaryFilterType>;
    {F::fromJson(json)} -> std::same_as<std::expected<nlohmann::json,std::string>>;
};

template<typename F,typename T>
concept BinaryFilter = fishnet::util::BiPredicate<F,T> && requires(const nlohmann::json & json) {
    {F::type()} -> std::same_as<BinaryFilterType>;
    {F::fromJson(json)} -> std::same_as<std::expected<F,std::string>>;
};

