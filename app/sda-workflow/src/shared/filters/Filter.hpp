//
// Created by lolo on 18.04.24.
//
#pragma once
#include <fishnet/FunctionalConcepts.hpp>
#include <string_view>
#include <optional>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fishnet/StaticMap.hpp>

enum class UnaryFilterType{
    ApproxAreaFilter, ProjectedAreaFilter
};

enum class BinaryFilterType{
    InsidePolygonFilter
};

template<typename F>
concept Filter = requires(const F & filter,const nlohmann::json & json) {
    {F::type()} -> std::same_as<UnaryFilterType>;
    {F::fromJson(json)} -> std::same_as<std::optional<F>>;
    {filter.toJson()} -> std::same_as<nlohmann::json>;
};

template<typename F,typename T>
concept UnaryFilter = fishnet::util::Predicate<F,T> && Filter<F>;

template<typename F,typename T>
concept BinaryFilter = fishnet::util::BiPredicate<F,T> && Filter<F>;
