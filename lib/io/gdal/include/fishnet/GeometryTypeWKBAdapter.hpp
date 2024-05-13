#pragma once
#include <fishnet/GeometryType.hpp>
#include <gdal/ogrsf_frmts.h>

namespace fishnet {
/**
 * @brief Static adaptor class to convert fishnet GeometryType <-> OGRwkbGeometryType
 * 
 */
class GeometryTypeWKBAdapter{
public:
    static OGRwkbGeometryType toWKB(geometry::GeometryType type){
        switch (type)
        {
            case geometry::GeometryType::POINT: return wkbPoint;
            case geometry::GeometryType::POLYGON: return wkbPolygon;
            case geometry::GeometryType::RING: return wkbLinearRing;
            case geometry::GeometryType::MULTIPOLYGON: return wkbMultiPolygon;
        default:
            throw std::invalid_argument("Geometry type: "+std::to_string(type)+" could not be converted into wkb format");
        }
    }
    static geometry::GeometryType fromWKB(OGRwkbGeometryType type){
        switch(type){
            case wkbPoint: return geometry::GeometryType::POINT;
            case wkbLinearRing: return geometry::GeometryType::RING;
            case wkbPolygon: return geometry::GeometryType::POLYGON;
            default: throw std::invalid_argument("wkbGeometryType could not be converted into a Fishnet geometry type");
        }
    }
};
}