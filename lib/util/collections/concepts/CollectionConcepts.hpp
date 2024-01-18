#pragma once
#include <ranges>
#include <concepts>
#include <algorithm>
namespace util{

template<typename R,typename T>
concept range_of = std::ranges::range<R> && std::convertible_to<std::ranges::range_value_t<R>,T>;

template<typename R,typename T>
concept forward_range_of =  std::ranges::forward_range<R> && range_of<R,T>;

template<typename R, typename T>
concept input_range_of = std::ranges::input_range<R> && range_of<R,T>;

template<typename R, typename T>
concept viewable_range_of = std::ranges::viewable_range<R> && range_of<R,T>;

template<typename R, typename T>
concept random_access_range_of = std::ranges::random_access_range<R> && range_of<R,T>;

template<typename V, typename T>
concept view_of = std::ranges::view<V> && range_of<V,T>;

template<std::ranges::range R>
constexpr size_t size(const R & range) noexcept{
    if constexpr(std::ranges::sized_range<R>){
        return std::ranges::distance(range);
    }else {
        return std::ranges::count_if(range, [](const auto & e){return true;});
    }
}

}