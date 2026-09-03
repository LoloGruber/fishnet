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

    /**
     * @brief Get the value of the Option if it has a value, otherwise throw an exception with the provided message.
     * 
     * @param message: The message to include in the exception if the Option is empty.
     * @return const T& The value of the Option.
     * @throws std::runtime_error if the Option is empty.
     */
    const T & value_or_throw(const std::string& message="No value present!") const& {
        if (this->has_value())
            return this->value();
        throw std::runtime_error(message);
    }

    /**
     * @brief Get the value of the Option if it has a value, otherwise throw an exception with the provided message.
     * 
     * @param message The message to include in the exception if the Option is empty. 
     * @return T&& The value of the Option.
     * @throws std::runtime_error if the Option is empty.
     */
    T&& value_or_throw(const std::string& message="No value present!") && {
        if (this->has_value())
            return std::move(this->value());
        throw std::runtime_error(message);
    }

    /**
     * @brief FlatMap the value of the Option if it has a value, otherwise return an empty Option
     *
     * @tparam F: (T) -> Option<U>
     * @param f flatMap function
     * @return Option<U>
     */
    template<typename F> requires fishnet::util::UnaryFunction<F,T,Option<typename std::remove_cvref_t<std::invoke_result_t<F,T>>::value_type>>
    Option<typename std::remove_cvref_t<std::invoke_result_t<F,T>>::value_type> and_then(F&& f) const& {
        using U = typename std::remove_cvref_t<std::invoke_result_t<F, const T&>>::value_type;
        if (this->has_value())
            return std::invoke(std::forward<F>(f), this->value());
        return Option<U>{};
    }

    /**
     * @brief FlatMap the value of the Option if it has a value, otherwise return an empty Option
     *
     * @tparam F: (T&&) -> Option<U>
     * @param f flatMap function
     * @return Option<U> &&
     */
    template<typename F> requires fishnet::util::UnaryFunction<F,T,Option<typename std::remove_cvref_t<std::invoke_result_t<F,T>>::value_type>>
    Option<typename std::remove_cvref_t<std::invoke_result_t<F,T>>::value_type> and_then(F&& f) && {
        using U = typename std::remove_cvref_t<std::invoke_result_t<F, T>>::value_type;
        if (this->has_value())
            return std::invoke(std::forward<F>(f), std::move(this->value()));
        return Option<U>{};
    }

    /**
     * @brief Transform the value of the Option if it has a value, otherwise return an empty Option
     * 
     * @tparam F: (T) -> U 
     * @param f map function
     * @return Option<U> 
     */
    template<typename F> requires fishnet::util::UnaryFunction<F,T,typename std::remove_cvref_t<std::invoke_result_t<F,T>>>
    Option<typename std::remove_cvref_t<std::invoke_result_t<F,T>>> transform(F&& f) const& {
        using U = std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
        if (this->has_value())
            return Option<U>(std::invoke(std::forward<F>(f), this->value()));
        return Option<U>{};
    }

    /**
     * @brief Transform the value of the Option if it has a value, otherwise return an empty Option
     * 
     * @tparam F: (T&&) -> U 
     * @param f map function
     * @return Option<U> &&
     */
    template<typename F> requires fishnet::util::UnaryFunction<F,T,typename std::remove_cvref_t<std::invoke_result_t<F,T>>>
    Option<typename std::remove_cvref_t<std::invoke_result_t<F,T>>> transform(F&& f) && {
        using U = std::remove_cvref_t<std::invoke_result_t<F, T>>;
        if (this->has_value())
            return Option<U>(std::invoke(std::forward<F>(f), std::move(this->value())));
        return Option<U>{};
    }

    /**
     * @brief If the Option has a value, return it. Otherwise, call the provided function and return its result.
     * 
     * @tparam F: () -> T
     * @param f function to call if the Option is empty
     * @return Option<T>
     */
    template<typename F> requires std::convertible_to<std::invoke_result_t<F>,T>
    Option<T> or_else(F&& f) const& {
        if (this->has_value()) return *this;
        return std::invoke(std::forward<F>(f));
    }

    /**
     * @brief If the Option has a value, return it. Otherwise, call the provided function and return its result.
     * 
     * @tparam F: () -> T
     * @param f function to call if the Option is empty
     * @return Option<T> &&
     */
    template<typename F> requires std::convertible_to<std::invoke_result_t<F>,Option<T>>
    Option<T> or_else(F&& f) && {
        if (this->has_value()) return std::move(*this);
        return std::invoke(std::forward<F>(f));
    }

    /**
     * @brief If the Option has a value and the predicate returns true, return the Option. Otherwise, return an empty Option.
     * 
     * @param predicate: (T) -> bool 
     * @return Option<T> 
     */
    Option<T> filter(fishnet::util::Predicate<T> auto const & predicate) {
        if (this->has_value() && predicate(this->value()))
            return *this;
        return Option<T>{};
    }

    /**
     * @brief If the Option has a value, call the provided function with the value. Otherwise, do nothing.
     * 
     * @param consumer: (T) -> void
     * @return void
     */
    void if_value(fishnet::util::Consumer<T> auto const & consumer) {
        if (this->has_value())
            consumer(std::forward<T>(this->value()));
    }

    /**
     * @brief If the Option has a value, call the provided function with the value. Otherwise, call the provided function.
     * 
     * @param consumer: (T) -> void 
     * @param runnable: () -> void
     * @return void
     */
    void if_value_or_else(fishnet::util::Consumer<T> auto const & consumer, fishnet::util::Runnable auto const & runnable) {
        if (this->has_value())
            consumer(std::forward<T>(this->value()));
        else
            runnable();
    }
};

template<typename T>
Option(std::optional<T> opt) -> Option<T>;

template<typename T>
Option(T value) -> Option<T>;

}  // namespace fishnet