#pragma once
#include <concepts>
namespace fishnet::math{

template<typename T>
concept Number = std::integral<T> || std::floating_point<T>;

template<typename From, typename To>
concept convertible_without_loss = requires(From f){To{f};};

using DEFAULT_FLOATING_POINT = double;
using DEFAULT_NUMERIC = double;
using DEFAULT_INTEGRAL = int;
}




