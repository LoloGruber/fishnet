#pragma once
#include <typeinfo>
#include <ogr_feature.h>
#include <variant>
#include <typeindex>
#include "FieldType.hpp"
namespace fishnet{

class OGRFieldAdapter{
public:
    static OGRFieldType fromTypeIndex(const std::type_index & type) noexcept {
        if (typeid(double) == type || typeid(float) == type)
            return OFTReal;
        if (typeid(int) == type || typeid(long) == type || typeid(size_t) == type)
            return OFTInteger64;
        return OFTString;
    }

    static void setFieldValue(OGRFeature * feature, const std::string & fieldName, const FieldType & value) noexcept {
        std::visit([&feature,&fieldName](auto && arg){
            using T = std::decay_t<decltype(arg)>;
            if constexpr(std::integral<T>)
                feature->SetField(fieldName.c_str(),GIntBig(( (long long) arg)));
        },value);

    }
};
}