#pragma once
#include <variant>
#include "NumericConcepts.hpp"

namespace fishnet {

using FieldType = std::variant<double,float,int,long,size_t,std::string,const char *>;

template<typename T>
concept FieldValueType = std::is_convertible_v<T,FieldType>;

}