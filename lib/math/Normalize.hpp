#pragma once
#include <cmath>
#include "NumericConcepts.hpp"
#include <type_traits>
#include <stdexcept>

namespace fishnet::math{

/**
 * Normalizes any value into the range: [0,upperBound)
*/
template<Number ValueType, Number BoundType>
static auto normalize(const ValueType value, BoundType upperBound){
    [[unlikely]] if (upperBound <= 0){
        throw std::invalid_argument("Cannot normalize value into range [0,0), upperBound is zero or less");
    }
    if constexpr(std::integral<ValueType> && std::integral<BoundType>){
        auto norm = value % upperBound;
        if (norm < 0){
            return norm + upperBound;
        }
        return norm;
    }else {
        auto norm = std::fmod(value,upperBound);
        if (norm < 0 ){
            return norm + upperBound;
        }
        return norm;
    }
};

static auto normalize(Number auto value, Number auto lowerBound, Number auto upperBound){
    [[unlikely]] if  (lowerBound >= upperBound){
        throw new std::invalid_argument("upperBound has to be greater than lowerBound");
    }
    auto range = upperBound - lowerBound;
    auto norm_to_range = normalize(value+lowerBound,range);
    return norm_to_range + lowerBound;
}
}