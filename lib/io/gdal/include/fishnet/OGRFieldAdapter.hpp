#pragma once
#include <typeinfo>
#include <ogr_feature.h>
#include <variant>
#include <typeindex>
#include <fishnet/FieldType.hpp>
#include <fishnet/FieldDefinition.hpp>
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

    template<typename T>
    static void setFieldValue(OGRFeature * feature, const std::string & fieldName, const T & value) noexcept {
            const char * name = fieldName.c_str();
            if constexpr(std::same_as<T,int>)
                feature->SetField(name, value);
            else if constexpr(std::integral<T>)
                feature->SetField(name, GIntBig((static_cast<long long>(value))));
            else if constexpr (std::floating_point<T>)
                feature->SetField(name, static_cast<double>(value));
            else if constexpr (std::same_as<T,std::string>)
                feature->SetField(name, value.c_str());
            else
                feature->SetField(name, value);
    }
};
}