#pragma once
#include <expected>
#include <stdexcept>
#include <functional>

#include <fishnet/FunctionalConcepts.hpp>

namespace fishnet{

template<typename T, typename E>
class Either: public std::expected<T,E>{
public:
    template<typename... Args>
    Either(Args&&... args): std::expected<T,E>(std::forward<Args>(args)...){}

    /**
     * @brief Get the value of the Either if it has a value, otherwise throw an exception using the error value if applicable, or a default message.
     * 
     * @return const T& the value of the Either.
     * @throws std::runtime_error if the Either is empty.
     */
    const T& value_or_throw() const&  {
        if(this->has_value())
            return this->value();
        if constexpr(std::convertible_to<E,std::string_view>){
            throw std::runtime_error(this->error());
        }else {
            throw std::runtime_error("No value present!");
        }
    }

    /**
     * @brief Get the value of the Either if it has a value, otherwise throw an exception using the error value if applicable, or a default message.
     * 
     * @return T& the value of the Either.
     * @throws std::runtime_error if the Either is empty.
     */
    T& value_or_throw() &  {
        if(this->has_value())
            return this->value();
        if constexpr(std::convertible_to<E,std::string_view>){
            throw std::runtime_error(this->error());
        }else {
            throw std::runtime_error("No value present!");
        }
    }  

    /**
     * @brief Get the value of the Either if it has a value, otherwise throw an exception using the error value if applicable, or a default message.
     * 
     * @return T&& the value of the Either.
     * @throws std::runtime_error if the Either is empty.
     */
    T&& value_or_throw() &&  {
        if(this->has_value())
            return std::move(this->value());
        if constexpr(std::convertible_to<E,std::string_view>){
            throw std::runtime_error(this->error());
        }else {
            throw std::runtime_error("No value present!");
        }
    }

    /**
     * @brief Get the value of the Either if it has a value, otherwise throw an exception with the provided message.
     * 
     * @param errorMsg: The message to include in the exception if the Either is empty.
     * @return const T& the value of the Either.
     * @throws std::runtime_error if the Either is empty.
     */
    const T& value_or_throw(const std::string & errorMsg) const&  {
        if(this->has_value())
            return this->value();
        else throw std::runtime_error(errorMsg);
    }   

    /**
     * @brief Get the value of the Either if it has a value, otherwise throw an exception with the provided message.
     * 
     * @param errorMsg: The message to include in the exception if the Either is empty.
     * @return T& the value of the Either.
     * @throws std::runtime_error if the Either is empty.
     */
    T& value_or_throw(const std::string & errorMsg) &  {
        if(this->has_value())
            return this->value();
        else throw std::runtime_error(errorMsg);
    }

    /**
     * @brief Get the value of the Either if it has a value, otherwise throw an exception with the provided message.
     * 
     * @param errorMsg: The message to include in the exception if the Either is empty.
     * @return T&& the value of the Either.
     * @throws std::runtime_error if the Either is empty.
     */
    T&& value_or_throw(const std::string & errorMsg) &&  {
        if(this->has_value())
            return std::move(this->value());
        else throw std::runtime_error(errorMsg);
    }

    /**
     * @brief flatMap the value of the Either if it has a value, otherwise return the error
     * 
     * @tparam F: (T) -> Either<U,E>
     * @param f flatMap function
     * @return Either<U,E> 
     */
    template<typename F> requires fishnet::util::UnaryFunction<F,T,Either<typename std::remove_cvref_t<std::invoke_result_t<F,T>>::value_type,E>>
    Either<typename std::remove_cvref_t<std::invoke_result_t<F,T>>::value_type,E> and_then(F&& f) & {
        using U = typename std::remove_cvref_t<std::invoke_result_t<F, const T&>>::value_type;
        if(this->has_value())
            return std::invoke(std::forward<F>(f), this->value());
        return Either<U,E>(std::unexpected(this->error()));
    }

    /**
     * @brief flatMap the value of the Either if it has a value, otherwise return the error
     * 
     * @tparam F: (T&&) -> Either<U,E>
     * @param f flatMap function
     * @return Either<U,E> 
     */
    template<typename F> requires fishnet::util::UnaryFunction<F,T,Either<typename std::remove_cvref_t<std::invoke_result_t<F,T>>::value_type,E>>
    Either<typename std::remove_cvref_t<std::invoke_result_t<F,T>>::value_type,E> and_then(F&& f) && {
        using U = typename std::remove_cvref_t<std::invoke_result_t<F, T>>::value_type;
        if(this->has_value())
            return std::invoke(std::forward<F>(f), std::move(this->value()));
        return Either<U,E>(std::unexpected(this->error()));
    }

    /**
     * @brief Transform the value of the Either if it has a value, otherwise return the error
     * 
     * @tparam F: (T) -> U 
     * @param f map function
     * @return Either<U,E> 
     */
    template<typename F> requires fishnet::util::UnaryFunction<F,T, typename std::remove_cvref_t<std::invoke_result_t<F,T>>>
    Either<typename std::remove_cvref_t<std::invoke_result_t<F,T>>,E> transform(F&& f) & {
        using U = typename std::remove_cvref_t<std::invoke_result_t<F, const T&>>;
        if(this->has_value())
            return std::invoke(std::forward<F>(f), this->value());
        return Either<U,E>(std::unexpected(this->error()));
    }

    /**
     * @brief Transform the value of the Either if it has a value, otherwise return the error
     * 
     * @tparam F: (T&&) -> U 
     * @param f map function
     * @return Either<U,E> 
     */
    template<typename F> requires fishnet::util::UnaryFunction<F,T, typename std::remove_cvref_t<std::invoke_result_t<F,T>>>
    Either<typename std::remove_cvref_t<std::invoke_result_t<F,T>>,E> transform(F&& f) && { 
        using U = typename std::remove_cvref_t<std::invoke_result_t<F, T>>;
        if(this->has_value())
            return std::invoke(std::forward<F>(f), std::move(this->value()));
        return Either<U,E>(std::unexpected(this->error()));
    }

    /**
     * @brief If the Either has a value, return it. Otherwise, call the provided function and return its result.
     * 
     * @tparam F: () -> Either<T,E>
     * @param f function to call if the Either is empty
     * @return Either<T,E>  
     */
    template<typename F> requires std::convertible_to<std::invoke_result_t<F>,Either<T,E>>
    Either<T,E> or_else(F&& f) & {
        if(this->has_value())
            return *this;
        return Either<T,E>(std::invoke(std::forward<F>(f)));
    }

    /**
     * @brief If the Either has a value, return it. Otherwise, call the provided function and return its result.
     * 
     * @tparam F: () -> Either<T,E>
     * @param f function to call if the Either is empty
     * @return Either<T,E> &&
     */
    template<typename F> requires std::convertible_to<std::invoke_result_t<F>,Either<T,E>>
    Either<T,E> or_else(F&& f) && {
        if(this->has_value())
            return std::move(*this);
        return Either<T,E>(std::invoke(std::forward<F>(f)));
    }

    /**
     * @brief If the Either has a value, call the provided function with the value. Otherwise, do nothing.
     * 
     * @param consumer: (T) -> void
     * @return void
     */
    void if_value(fishnet::util::Consumer<T> auto const & consumer){
        if(this->has_value())
            consumer(std::forward<T>(this->value()));
    }

    /**
     * @brief If the Either has a value, call the provided function with the value. Otherwise, call the provided function with the error.
     * 
     * @param consumer: (T) -> void
     * @param errorConsumer: (E) -> void
     * @return void
     */
    void if_value_or_error(fishnet::util::Consumer<T> auto const & consumer, fishnet::util::Consumer<E> auto const & errorConsumer){
        if(this->has_value())
            consumer(std::forward<T>(this->value()));
        else
            errorConsumer(std::forward<E>(this->error()));
    }
};
}