#pragma once
#include <expected>
#include <stdexcept>
#include <string>
namespace fishnet::util{

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
};
}