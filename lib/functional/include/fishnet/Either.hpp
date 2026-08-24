#pragma once
#include <expected>
#include <stdexcept>
#include <functional>

namespace fishnet{

template<typename T, typename E>
class Either: public std::expected<T,E>{
public:
    template<typename... Args>
    Either(Args&&... args): std::expected<T,E>(std::forward<Args>(args)...){}

    T value_or_throw() &  {
        if(this->has_value())
            return this->value();
        if constexpr(std::convertible_to<E,std::string_view>){
            throw std::runtime_error(this->error());
        }else {
            throw std::runtime_error("No value present!");
        }
    }

    T&& value_or_throw() &&  {
        if(this->has_value())
            return std::move(this->value());
        if constexpr(std::convertible_to<E,std::string_view>){
            throw std::runtime_error(this->error());
        }else {
            throw std::runtime_error("No value present!");
        }
    }

    T value_or_throw(const std::string & errorMsg) &  {
        if(this->has_value())
            return this->value();
        else throw std::runtime_error(errorMsg);
    }

    T&& value_or_throw(const std::string & errorMsg) &&  {
        if(this->has_value())
            return std::move(this->value());
        else throw std::runtime_error(errorMsg);
    }

    template<typename F> requires std::convertible_to<std::invoke_result_t<F>,Either<T,E>>
    Either<T,E> or_else(F&& f) & {
        if(this->has_value())
            return *this;
        return Either<T,E>(std::invoke(std::forward<F>(f)));
    }

    template<typename F> requires std::convertible_to<std::invoke_result_t<F>,Either<T,E>>
    Either<T,E> or_else(F&& f) && {
        if(this->has_value())
            return std::move(*this);
        return Either<T,E>(std::invoke(std::forward<F>(f)));
    }
};
}