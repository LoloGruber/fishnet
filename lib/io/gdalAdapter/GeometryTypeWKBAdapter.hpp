#pragma once
#include "GeometryType.hpp"
#include <ogrsf_frmts.h>

namespace fishnet {
using namespace fishnet::geometry;
class GeometryTypeWKBAdapter{
public:
static OGRwkbGeometryType toWKB(geometry::GeometryType type){
    switch (type)
    {
        case GeometryType::POINT: return wkbPoint;
        case GeometryType::POLYGON: return wkbPolygon;
        case GeometryType::RING: return wkbLinearRing;
        case GeometryType::MULTIPOLYGON: return wkbMultiPolygon;
    default:
        throw std::invalid_argument("Geometry type: "+std::to_string(type)+" could not be converted into wkb format");
    }
}
static geometry::GeometryType fromWKB(OGRwkbGeometryType type){
    switch(type){
        case wkbPoint: return GeometryType::POINT;
        case wkbLinearRing: return GeometryType::RING;
        case wkbPolygon: return GeometryType::POLYGON;
        default: throw std::invalid_argument("wkbGeometryType could not be converted into a Fishnet geometry type");
    }
}
};
}