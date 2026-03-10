#pragma once
#include <cmath>
#include <ranges>
#include <algorithm>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/Option.hpp>
#include <type_traits>

namespace fishnet::math{

static Option<double> mean(std::ranges::input_range auto && values, fishnet::util::UnaryFunction<std::ranges::range_value_t<decltype(values)>, double> auto const& valueMapper){
    if(fishnet::util::isEmpty(values)){
        return std::nullopt;
    }
    double count = static_cast<double>(fishnet::util::size(values));
    return std::ranges::fold_left(values, 0.0, [&](double acc, const auto & value) {
        return acc + std::invoke(valueMapper, value);
    }) / count;
}

static Option<double> mean(std::ranges::input_range auto && values) requires std::is_convertible_v<double,std::ranges::range_value_t<decltype(values)>>{
    return mean(values, [](const auto & value){
        return static_cast<double>(value);
    });
}

static Option<double> avg(std::ranges::input_range auto && values, fishnet::util::UnaryFunction<std::ranges::range_value_t<decltype(values)>, double> auto const& valueMapper){
    return mean(values, valueMapper);
} 

static Option<double> avg(std::ranges::input_range auto && values) requires std::is_convertible_v<double,std::ranges::range_value_t<decltype(values)>>{
    return mean(values, [](const auto & value){
        return static_cast<double>(value);
    });
}

static Option<double> var(std::ranges::input_range auto  && values, fishnet::util::UnaryFunction<std::ranges::range_value_t<decltype(values)>, double> auto const& valueMapper, double meanValue){
    if(fishnet::util::isEmpty(values)){
        return std::nullopt;
    }
    double count = static_cast<double>(fishnet::util::size(values));
    return std::ranges::fold_left(values, 0.0, [&](double acc, const auto & value) {
        double diff_to_mean = std::invoke(valueMapper, value) - meanValue;
        return acc + diff_to_mean * diff_to_mean;
    }) / count;
} 

static Option<double> var(std::ranges::input_range auto && values, fishnet::util::UnaryFunction<std::ranges::range_value_t<decltype(values)>, double> auto const& valueMapper){
    return mean(values,valueMapper).and_then([&](double meanValue){
        return var(values, valueMapper, meanValue);
    });
}

static Option<double> var(std::ranges::input_range auto && values) requires std::is_convertible_v<double,std::ranges::range_value_t<decltype(values)>>{
    return var(values, [](const auto & value){
        return static_cast<double>(value);
    });
}

static Option<double> var(std::ranges::input_range auto && values, double mean) requires std::is_convertible_v<double,std::ranges::range_value_t<decltype(values)>>{
    return var(values, [](const auto & value){
        return static_cast<double>(value);
    }, mean);
}

static Option<double> std(std::ranges::input_range auto && values, fishnet::util::UnaryFunction<std::ranges::range_value_t<decltype(values)>, double> auto const& valueMapper, double meanValue){
    return var(values, valueMapper, meanValue).transform([](double variance){
        return sqrt(variance);
    });
}


static Option<double> std(std::ranges::input_range auto && values, fishnet::util::UnaryFunction<std::ranges::range_value_t<decltype(values)>, double> auto const& valueMapper){
    return mean(values,valueMapper).and_then([&](double meanValue){
        return std(values, valueMapper, meanValue);
    });
}

static Option<double> std(std::ranges::input_range auto && values) requires std::is_convertible_v<double,std::ranges::range_value_t<decltype(values)>>{
    return std(values, [](const auto & value){
        return static_cast<double>(value);
    });
}

static Option<double> std(std::ranges::input_range auto && values, double mean) requires std::is_convertible_v<double,std::ranges::range_value_t<decltype(values)>>{
    return std(values, [](const auto & value){
        return static_cast<double>(value);
    }, mean);
}

}// namespace fishnet::math
