#pragma once
#include <gdal/gdal.h>
#include <gdal/ogr_geometry.h>

#include <optional>
#include <fishnet/GeometryObject.hpp>
#include <fishnet/Vec2D.hpp>
#include <fishnet/Ring.hpp>
#include <fishnet/Polygon.hpp>
#include <fishnet/SimplePolygon.hpp>
#include <fishnet/MultiPolygon.hpp>

namespace fishnet {
/**
 * @brief Static geometry adaptor: fishnet::Geometry <-> OGRGeometry
 * 
 */
class OGRGeometryAdapter{
public: 
template<typename T>
using OGRUniquePtr = std::unique_ptr<T>;

static fishnet::geometry::Vec2D<fishnet::math::DEFAULT_NUMERIC> fromOGR(const OGRPoint & ogrPoint) noexcept {
    return fishnet::geometry::Vec2D(ogrPoint.getX(), ogrPoint.getY());
}

template<fishnet::math::Number T>
static OGRUniquePtr<OGRPoint> toOGR(const fishnet::geometry::Vec2D<T> & point) noexcept {
    return OGRUniquePtr<OGRPoint>(new OGRPoint(double(point.x),double(point.y)));
}

static std::optional<fishnet::geometry::Ring<fishnet::math::DEFAULT_NUMERIC>> fromOGR(const OGRLinearRing& ogrRing) noexcept {
    std::vector<fishnet::geometry::Vec2D<fishnet::math::DEFAULT_NUMERIC>> pointsInOrder;
    for(const auto & ogrPoint : ogrRing){
        pointsInOrder.push_back(fromOGR(ogrPoint));
    }
    try{
        return fishnet::geometry::Ring<fishnet::math::DEFAULT_NUMERIC>(pointsInOrder);
    }catch(std::invalid_argument & exception){
        return std::nullopt;
    }
}

static OGRUniquePtr<OGRLinearRing> toOGR(fishnet::geometry::IRing auto const & ring ) noexcept {
    OGRUniquePtr<OGRLinearRing> ogrRing {new OGRLinearRing()};
    auto && points = ring.getPoints();
    for(const auto & p : points) {
        ogrRing->addPoint(toOGR(p).get());
    }
    auto firstPoint = *std::ranges::begin(points);
    ogrRing->addPoint(toOGR(firstPoint).get());
    return ogrRing;
}

static std::optional<fishnet::geometry::Polygon<fishnet::math::DEFAULT_NUMERIC>> fromOGR(const OGRPolygon & ogrPolygon) noexcept {
    try{
        auto ogrBoundary = ogrPolygon.getExteriorRing();
        auto fishnetBoundary = fromOGR(*ogrBoundary);
        if(not fishnetBoundary) return std::nullopt;
        std::vector<fishnet::geometry::Ring<fishnet::math::DEFAULT_NUMERIC>> holes;
        for(int i = 0; i < ogrPolygon.getNumInteriorRings(); i++){
            auto hole = fromOGR(*(ogrPolygon.getInteriorRing(i)));
            if(not hole) return std::nullopt;
            holes.push_back(hole.value());
        }
        return fishnet::geometry::Polygon<fishnet::math::DEFAULT_NUMERIC>(fishnetBoundary.value(),holes);
    }catch(fishnet::geometry::InvalidGeometryException & e){
        return std::nullopt;
    }
}

static OGRUniquePtr<OGRPolygon> toOGR(fishnet::geometry::IPolygon auto const & polygon) noexcept {
    OGRUniquePtr<OGRPolygon> ogrPoly {new OGRPolygon()};
    ogrPoly->addRing(toOGR(polygon.getBoundary())->toCurve());
    for(const auto & ring : polygon.getHoles()) {
        ogrPoly->addRing(toOGR(ring)->toCurve());
    }
    return ogrPoly;
}

static OGRUniquePtr<OGRMultiPolygon> toOGR(fishnet::geometry::IMultiPolygon auto const & multiPolygon) noexcept {
    OGRUniquePtr<OGRMultiPolygon> ogrMulti {new OGRMultiPolygon()};
    for(const auto & polygon: multiPolygon.getPolygons()){
        ogrMulti->addGeometry(OGRGeometryFactory::forceToPolygon(toOGR(polygon).get()));
    }
    return ogrMulti; //https://gis.stackexchange.com/questions/297251/ogr-multipolygon-with-holes-model TODO
    // https://en.wikipedia.org/wiki/Well-known_text_representation_of_geometry maybe change direction of inner rings
}

static std::optional<fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<fishnet::math::DEFAULT_NUMERIC>>> fromOGR(const OGRMultiPolygon & multiPolygon) noexcept {
    try{
        
        std::vector<fishnet::geometry::Polygon<fishnet::math::DEFAULT_NUMERIC>> polygons;
        for(auto ogrPolygonPtr : multiPolygon) {
            auto polygon = fromOGR(*ogrPolygonPtr);
            if (not polygon)
                return std::nullopt;
            polygons.push_back(polygon.value());
        }
        return fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<double>>(polygons);
    }catch(fishnet::geometry::InvalidGeometryException & ex) {
        return std::nullopt;
    }
}


template<fishnet::geometry::GeometryType G>
constexpr static auto fromOGR(const OGRGeometry & ogrGeometry){
    if constexpr(G == fishnet::geometry::GeometryType::POLYGON){
        return fromOGR(*ogrGeometry.toPolygon());
    } else if constexpr(G == fishnet::geometry::GeometryType::POINT){
        return fromOGR(*ogrGeometry.toPoint());
    } else if constexpr(G == fishnet::geometry::GeometryType::RING){
        return fromOGR(*ogrGeometry.toLinearRing());
    }else if constexpr(G == fishnet::geometry::GeometryType::MULTIPOLYGON){
        return fromOGR(*ogrGeometry.toMultiPolygon());
    }else {
        return std::optional<fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<double>>>();
    }
}

};
}