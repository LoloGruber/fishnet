#pragma once
#include <variant>
#include "NumericConcepts.hpp"

namespace fishnet {

using FieldType = std::variant<double,float,int,long,size_t,std::string,const char *>;

template<typename T>
concept FieldValueType = math::Number<T> || std::convertible_to<T,std::string>;

}