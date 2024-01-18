#pragma once
#include "NumericConcepts.hpp"
#include <cmath>

namespace fishnet::math{
    constexpr static double PI = 3.14159265358979323846;
    constexpr static double TWO_PI = 2*PI; 
    constexpr static double DEG_TO_RAD = PI / 180.0;
    constexpr static double RAD_TO_DEG = 180.0 / PI;
    constexpr static double DOUBLE_EPSILON = 1e-8;
    constexpr static float FLOAT_EPSILON=1e-5f;

    template<std::floating_point F>
    constexpr static DEFAULT_FLOATING_POINT getDefaultEpsilon(){
        if constexpr(std::same_as<DEFAULT_FLOATING_POINT,double>) return DOUBLE_EPSILON;
        if constexpr(std::same_as<DEFAULT_FLOATING_POINT,float>) return FLOAT_EPSILON;
        else return std::numeric_limits<F>::epsilon();
    }

    constexpr static DEFAULT_FLOATING_POINT EPSILON = getDefaultEpsilon<DEFAULT_FLOATING_POINT>();

    template<Number T>
    constexpr inline bool isZero(T number) noexcept{
        if constexpr(std::integral<T>) return number == 0;
        if constexpr(std::same_as<double,T>) return fabs(number) < DOUBLE_EPSILON;
        if constexpr(std::same_as<float,T>) return fabs(number) <FLOAT_EPSILON;
        return number==0; //default case
    }

    template<Number T, Number U>
    constexpr inline bool areEqual(T lhs, U rhs) noexcept {
        if constexpr(std::integral<T> && std::integral<U>) return lhs==rhs;
        if constexpr(std::integral<T> && std::floating_point<U>) return isZero(U(lhs)-rhs);
        if constexpr(std::integral<U> && std::floating_point<T>) return isZero(T(rhs)-lhs);
        if constexpr(std::floating_point<T> && std::floating_point<U>) return isZero(lhs-rhs);
        
    }

}