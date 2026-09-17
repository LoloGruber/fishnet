#pragma once
#include <fishnet/Concepts.hpp>

namespace fishnet::geometry{

enum class GeometryType {
    POINT,SEGMENT,RAY,LINE,RING,POLYGON,MULTIPOLYGON
};

template<typename G, typename T = typename std::remove_cvref_t<G>::numeric_type>
concept GeometryBase = fishnet::math::Number<T> 
    && fishnet::util::Object<G>
    && requires(){
        {std::remove_cvref_t<G>::type} -> std::convertible_to<GeometryType>;
        typename std::remove_cvref_t<G>::numeric_type;
    };
};


#include <string>
namespace std {
    static string to_string(fishnet::geometry::GeometryType type) noexcept {
        switch(type) {
            case fishnet::geometry::GeometryType::POINT: return "POINT";
            case fishnet::geometry::GeometryType::SEGMENT: return "SEGMENT";
            case fishnet::geometry::GeometryType::RAY: return "RAY";
            case fishnet::geometry::GeometryType::LINE: return "LINE";
            case fishnet::geometry::GeometryType::RING: return "RING";
            case fishnet::geometry::GeometryType::POLYGON: return "POLYGON";
            case fishnet::geometry::GeometryType::MULTIPOLYGON: return "MULTIPOLYGON";
            default: return "undefined";
        }
    }
}