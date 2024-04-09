//
// Created by lolo on 09.04.24.
//
#pragma once
#include <ranges>
#include <algorithm>
#include "FunctionalConcepts.hpp"

namespace util{

/**
 * Base class for composite Predicates
 * @tparam Types argument types for the predicate
 */
template<typename... Types>
class CompositePredicate{
protected:
    using predicate_type = std::function<bool(Types...)>;
    std::vector<predicate_type> predicates;

    constexpr explicit CompositePredicate(predicate_type && predicate){
        predicates.push_back(predicate);
    }

    constexpr explicit CompositePredicate(predicate_type predicate){
        predicates.push_back(std::move(predicate));
    }

    constexpr CompositePredicate() = default;
public:
    constexpr void add(predicate_type && predicate) {
        predicates.push_back(predicate);
    }

    constexpr void add(const predicate_type & predicate) {
        predicates.push_back(std::move(predicate));
    }
};

/**
 * All predicates have to match for the composite predicate to become true
 * @tparam Types argument types for the predicate
 */
template<typename... Types>
class AllOfPredicate:public CompositePredicate<Types...>{
private:
    using Parent = CompositePredicate<Types...>;
public:
    constexpr AllOfPredicate()=default;

    constexpr explicit AllOfPredicate(Parent::predicate_type && predicate):Parent(predicate){}

    constexpr explicit AllOfPredicate(Parent::predicate_type  predicate):Parent(predicate){}

    constexpr bool operator()(Types... args) const noexcept{
        return std::ranges::all_of(this->predicates,[... args = std::forward<Types>(args)](const auto & predicate){
            return predicate(args...);
        });
    }
};

/**
 * Any predicate has to match for the composite predicate to become true
 * @tparam Types argument types for the predicate
 */
template<typename... Types>
class AnyOfPredicate:public CompositePredicate<Types...>{
private:
    using Parent = CompositePredicate<Types...>;
public:
    constexpr AnyOfPredicate()=default;

    constexpr explicit AnyOfPredicate(Parent::predicate_type && predicate):Parent(predicate){}

    constexpr explicit AnyOfPredicate(Parent::predicate_type  predicate):Parent(predicate){}

    constexpr bool operator()(Types... args) const noexcept{
        return std::ranges::any_of(this->predicates,[... args = std::forward<Types>(args)] (const auto & predicate){
            return predicate(args...);
        });
    }
};
static_assert(util::Predicate<AnyOfPredicate<int>,int>);
static_assert(util::BiPredicate<AllOfPredicate<int,double>,int,double>);
}
