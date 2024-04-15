#pragma once
#include <concepts>
namespace fishnet::math{

template<typename T>
concept Number = std::integral<T> || std::floating_point<T>;

using DEFAULT_FLOATING_POINT = double;
using DEFAULT_NUMERIC = double;
using DEFAULT_INTERGRAL = int;
}




