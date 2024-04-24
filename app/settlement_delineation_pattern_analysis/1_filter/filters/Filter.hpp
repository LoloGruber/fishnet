//
// Created by lolo on 18.04.24.
//
#pragma once
#include <fishnet/FunctionalConcepts.hpp>
#include <string_view>
#include <expected>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fishnet/BidirectionalMap.hpp>

enum class UnaryFilterType{
    ApproxAreaFilter, ProjectedAreaFilter
};

enum class BinaryFilterType{
    InsideBoundaryFilter
};

namespace FILTERS{

const static inline util::BidirectionalMap<std::string,UnaryFilterType> UNARY = {{
    {"ApproxAreaFilter",UnaryFilterType::ApproxAreaFilter},
    {"ProjectedAreaFilter",UnaryFilterType::ProjectedAreaFilter}
}};



const static inline util::BidirectionalMap<std::string,BinaryFilterType> BINARY = {
    {"InsideBoundaryFilter",BinaryFilterType::InsideBoundaryFilter}
};
}


template<typename F,typename T>
concept UnaryFilter = util::Predicate<F,T> && requires(const nlohmann::json & json) {
    {F::type()} -> std::same_as<UnaryFilterType>;
    {F::fromJson(json)} -> std::same_as<std::expected<nlohmann::json,std::string>>;
};

template<typename F,typename T>
concept BinaryFilter = util::BiPredicate<F,T> && requires(const nlohmann::json & json) {
    {F::type()} -> std::same_as<BinaryFilterType>;
    {F::fromJson(json)} -> std::same_as<std::expected<F,std::string>>;
};

