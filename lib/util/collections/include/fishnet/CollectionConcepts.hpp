#pragma once
#include <ranges>
#include <concepts>
#include <algorithm>
namespace fishnet::util{
/**
 * @brief Metaprogramming Helper which checks whether a concept is satisfied
 * (Futher information can found here: https://www.youtube.com/watch?v=KENynEQoqCo)
 * Usage: Satisfies<int,[]<std::integral>{}>
 * @tparam T type to test
 * @tparam C concept to be fulfilled
 */
template<auto C, typename T>
concept Satisfies = requires {
    C.template operator()<T>();
};

template<typename R,typename T>
concept range_of = std::ranges::range<R> && std::convertible_to<std::ranges::range_value_t<R>,T>;

template<typename R, auto C>
concept range_over = std::ranges::range<R> && Satisfies<C,std::ranges::range_value_t<R>>;

template<typename R,typename T>
concept forward_range_of = std::ranges::forward_range<R> && range_of<R,T>;

template<typename R, auto C>
concept forward_range_over = std::ranges::forward_range<R> && range_over<R,C>;

template<typename R, typename T>
concept input_range_of = std::ranges::input_range<R> && range_of<R,T>;

template<typename R,auto C>
concept input_range_over = std::ranges::input_range<R> && range_over<R,C>;

template<typename R, typename T>
concept random_access_range_of = std::ranges::random_access_range<R> && range_of<R,T>;

template<typename R, auto C>
concept random_access_range_over = std::ranges::random_access_range<R> && range_over<R,C>;

template<typename V, typename T>
concept view_of = std::ranges::view<V> && range_of<V,T>;

template<typename V, auto C>
concept view_over = std::ranges::view<V> && range_over<V,C>;

constexpr size_t size(std::ranges::range auto && range) noexcept{
    using R = decltype(range);
    if constexpr(std::ranges::sized_range<R>){
        return std::ranges::distance(range);
    }else {
        return std::ranges::count_if(range, [](const auto & e){return true;});
    }
}
}