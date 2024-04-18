//
// Created by lolo on 18.04.24.
//
#pragma once
#include <fishnet/FunctionalConcepts.hpp>

enum class UnaryFilterType{
    ApproxAreaFilter, ProjectedAreaFilter
};

enum class BinaryFilterType{
    InsideBoundaryFilter
};

template<typename FilterType,typename T>
concept UnaryFilter = util::Predicate<FilterType,T> && requires (const FilterType & f){
    {f.getType()} -> std::same_as<UnaryFilterType>;
};

template<typename FilterType, typename T>
concept BinaryFilter = util::BiPredicate<FilterType,T> && requires(const FilterType & f){
    {f.getType()} -> std::same_as<BinaryFilterType>;
};
