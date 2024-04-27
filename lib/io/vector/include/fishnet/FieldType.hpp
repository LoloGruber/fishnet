#pragma once
#include <variant>
#include <fishnet/NumericConcepts.hpp>

namespace fishnet {

using FieldType = std::variant<double,float,int,long,size_t,long long,std::string,const char *>;

template<typename T>
concept FieldValueType = std::is_convertible_v<T,FieldType>;

}