#pragma once
#include <functional>
#include <concepts>
#include <ranges>

namespace fishnet::util{

template<typename F, typename T, typename R>
concept UnaryFunction = std::convertible_to<F,std::function<R(const T &)>>;

template<typename F, typename T, typename U,typename R>
concept BiFunction = std::convertible_to<F,std::function<R(const T &, const U &)>>;

template<typename F, typename T>
concept UnaryOperator = UnaryFunction<F,T,T>;

template<typename F, typename T>
concept BiOperator = BiFunction<F,T,T,T>;

template<typename F, typename T>
concept Predicate = UnaryFunction<F,T,bool>;

template<typename T>
using Predicate_t = std::function<bool(const T &)>;

template<typename F,typename T,typename R= T>
concept BiPredicate = BiFunction<F,T,R,bool>;

template<typename T, typename  R = T>
using BiPredicate_t = std::function<bool(const T &, const R &)>;

template<typename F, typename T>
concept Producer = std::convertible_to<F,std::function<T()>>;

template<typename F, typename T>
concept Consumer = UnaryFunction<F,T,void>;

template<typename F, typename R>
concept ReduceFunction = std::ranges::forward_range<R> && UnaryFunction<F,R,std::ranges::range_value_t<R>>;

struct TruePredicate{
    bool inline operator()(const auto & t)const noexcept {
        return true;
    }
};

struct FalsePredicate {
    bool inline operator()(const auto & t) const noexcept {
        return false;
    }
};

struct TrueBiPredicate{
    bool inline operator()(const auto & lhs, const auto & rhs) const noexcept {
        return true;
    }
};

struct FalseBiPredicate {
    bool inline operator()(const auto & lhs, const auto & rhs) const noexcept {
        return false;
    }
};


class LimitPredicate {
private:
    const size_t limit;
    size_t counter;
public:
    LimitPredicate(size_t limit):limit(limit),counter(0){}
    bool operator()(const auto & t) noexcept {
        return counter++ < limit;
    }
};
}