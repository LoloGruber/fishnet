#pragma once

template<typename T>
concept Printable = requires(const T & obj){
    {obj.toString()} -> std::convertible_to<std::string>;
};

constexpr std::ostream & operator << (std::ostream & os,Printable auto const & obj) noexcept{
    os << obj.toString();
    return os;
}

