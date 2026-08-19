#pragma once
#include <fishnet/FunctionalConcepts.hpp>
#include <concepts>
#include <optional>
#include <string>
#include <functional>
#include <stdexcept>
#include <type_traits>

namespace fishnet {

template<typename T>
class Option : public std::optional<T> {
public:
    template<typename... Args>
    Option(Args&&... args) : std::optional<T>(std::forward<Args>(args)...) {}

    const T & value_or_throw(const std::string& message="No value present!") const& {
        if (this->has_value())
            return this->value();
        throw std::runtime_error(message);
    }

    T&& value_or_throw(const std::string& message="No value present!") && {
        if (this->has_value())
            return std::move(this->value());
        throw std::runtime_error(message);
    }

    /* F: T -> Option<U>*/
    template<typename F>
    auto and_then(F&& f) const& {
        using U = typename std::remove_cvref_t<std::invoke_result_t<F, const T&>>::value_type;
        if (this->has_value())
            return Option<U>(std::invoke(std::forward<F>(f), this->value()));
        return Option<U>{};
    }

    template<typename F>
    auto and_then(F&& f) && {
        using U = typename std::remove_cvref_t<std::invoke_result_t<F, T>>::value_type;
        if (this->has_value())
            return Option<U>(std::invoke(std::forward<F>(f), std::move(this->value())));
        return Option<U>{};
    }

    /* F: T ->  U */
    template<typename F>
    auto transform(F&& f) const& {
        using U = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
        if (this->has_value())
            return Option<U>(std::invoke(std::forward<F>(f), this->value()));
        return Option<U>{};
    }

    template<typename F>
    auto transform(F&& f) && {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T>>;
        if (this->has_value())
            return Option<U>(std::invoke(std::forward<F>(f), std::move(this->value())));
        return Option<U>{};
    }

    /* F: () -> T */
    template<typename F> requires std::convertible_to<std::invoke_result_t<F>,T>
    Option<T> or_else(F&& f) const& {
        if (this->has_value()) return *this;
        return std::invoke(std::forward<F>(f));
    }

    template<typename F> requires std::convertible_to<std::invoke_result_t<F>,Option<T>>
    Option<T> or_else(F&& f) && {
        if (this->has_value()) return std::move(*this);
        return std::invoke(std::forward<F>(f));
    }

    auto filter(fishnet::util::Predicate<T> auto const & predicate) {
        if (this->has_value() && predicate(this->value()))
            return *this;
        return Option<T>{};
    }
};

template<typename T>
Option(std::optional<T> opt) -> Option<T>;

template<typename T>
Option(T value) -> Option<T>;

}  // namespace fishnet