#pragma once
#include <gdal/gdal.h>
#include <gdal/ogr_geometry.h>

#include <optional>
#include "GeometryObject.hpp"

#include "Vec2D.hpp"
#include "Ring.hpp"
#include "Polygon.hpp"
#include "SimplePolygon.hpp"
#include "MultiPolygon.hpp"

using namespace fishnet::geometry;

namespace fishnet::geometry::__impl{
template<typename G>
concept ISimplePolygon = fishnet::geometry::IRing<G> && fishnet::geometry::IPolygon<G>;
}
namespace fishnet {


class OGRGeometryAdapter{
public: 
template<typename T>
using OGRUniquePtr = std::unique_ptr<T>;


static Vec2D<fishnet::math::DEFAULT_NUMERIC> fromOGR(const OGRPoint & ogrPoint) noexcept {
    return Vec2D(ogrPoint.getX(), ogrPoint.getY());
}

template<fishnet::math::Number T>
static OGRUniquePtr<OGRPoint> toOGR(const Vec2D<T> & point) noexcept {
    return OGRUniquePtr<OGRPoint>(new OGRPoint(double(point.x),double(point.y)));
}

static std::optional<Ring<fishnet::math::DEFAULT_NUMERIC>> fromOGR(const OGRLinearRing& ogrRing) noexcept {
    std::vector<Vec2D<fishnet::math::DEFAULT_NUMERIC>> pointsInOrder;
    for(const auto & ogrPoint : ogrRing){
        pointsInOrder.push_back(fromOGR(ogrPoint));
    }
    try{
        return Ring<fishnet::math::DEFAULT_NUMERIC>(pointsInOrder);
    }catch(std::invalid_argument & exception){
        return std::nullopt;
    }
}



static OGRUniquePtr<OGRPolygon> toOGR(fishnet::geometry::__impl::ISimplePolygon auto const & polygon) noexcept {
    OGRUniquePtr<OGRPolygon> ogrPoly {new OGRPolygon()};
    ogrPoly->addRing(toOGR(polygon.getBoundary())->toCurve());
    return ogrPoly;
}


static OGRUniquePtr<OGRLinearRing> toOGR(IRing auto const & ring ) noexcept {
    OGRUniquePtr<OGRLinearRing> ogrRing {new OGRLinearRing()};
    auto && points = ring.getPoints();
    for(const auto & p : points) {
        ogrRing->addPoint(toOGR(p).get());
    }
    auto firstPoint = *std::ranges::begin(points);
    ogrRing->addPoint(toOGR(firstPoint).get());
    return ogrRing;
}

static std::optional<Polygon<fishnet::math::DEFAULT_NUMERIC>> fromOGR(const OGRPolygon & ogrPolygon) noexcept {
    try{
        auto ogrBoundary = ogrPolygon.getExteriorRing();
        auto fishnetBoundary = fromOGR(*ogrBoundary);
        if(not fishnetBoundary) return std::nullopt;
        std::vector<Ring<fishnet::math::DEFAULT_NUMERIC>> holes;
        for(int i = 0; i < ogrPolygon.getNumInteriorRings(); i++){
            auto hole = fromOGR(*(ogrPolygon.getInteriorRing(i)));
            if(not hole) return std::nullopt;
            holes.push_back(hole.value());
        }
        return Polygon<fishnet::math::DEFAULT_NUMERIC>(fishnetBoundary.value(),holes);
    }catch(InvalidGeometryException & e){
        return std::nullopt;
    }
}

static OGRUniquePtr<OGRPolygon> toOGR(IPolygon auto const & polygon) noexcept {
    OGRUniquePtr<OGRPolygon> ogrPoly {new OGRPolygon()};
    ogrPoly->addRing(toOGR(polygon.getBoundary())->toCurve());
    for(const auto & ring : polygon.getHoles()) {
        ogrPoly->addRing(toOGR(ring)->toCurve());
    }
    return ogrPoly;
}

static OGRUniquePtr<OGRMultiPolygon> toOGR(IMultiPolygon auto const & multiPolygon) noexcept {
    OGRUniquePtr<OGRMultiPolygon> ogrMulti {new OGRMultiPolygon()};
    for(const auto & polygon: multiPolygon.getPolygons()){
        ogrMulti->addGeometry(OGRGeometryFactory::forceToPolygon(toOGR(polygon).get()));
    }
    return ogrMulti; //https://gis.stackexchange.com/questions/297251/ogr-multipolygon-with-holes-model TODO
    // https://en.wikipedia.org/wiki/Well-known_text_representation_of_geometry maybe change direction of inner rings
}

template<GeometryType G>
constexpr static fishnet::geometry::OptionalGeometryObject auto fromOGR(const OGRGeometry & ogrGeometry){
    if constexpr(G == GeometryType::POLYGON){
        return fromOGR(*ogrGeometry.toPolygon());
    } else if constexpr(G == GeometryType::POINT){
        return fromOGR(*ogrGeometry.toPoint());
    } else if constexpr(G == GeometryType::RING){
        return fromOGR(*ogrGeometry.toLinearRing());
    }else {
        return std::optional<fishnet::geometry::MultiPolygon<Polygon<double>>>();
    }
}

};
}