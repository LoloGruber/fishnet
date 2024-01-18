#pragma once

namespace fishnet::geometry {
enum class GeometryType {
    POINT,SEGMENT,RAY,LINE,RING,POLYGON,MULTIPOLYGON
};
}

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