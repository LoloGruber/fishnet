#pragma once
#include <gdal/gdal.h>
#include <gdal/ogr_geometry.h>
#include <spdlog/spdlog.h>
#include <fishnet/Option.hpp>
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
private:
    struct OGRGeometryDeleter {
        void operator()(OGRGeometry* geom) const {
            OGRGeometryFactory::destroyGeometry(geom);
        }
    };
public: 
template<typename T>
using OGRUniquePtr = std::unique_ptr<T, OGRGeometryDeleter>;

static fishnet::geometry::Vec2D<fishnet::math::DEFAULT_NUMERIC> fromOGR(const OGRPoint & ogrPoint) noexcept {
    return fishnet::geometry::Vec2D(ogrPoint.getX(), ogrPoint.getY());
}

template<fishnet::math::Number T>
static OGRUniquePtr<OGRPoint> toOGR(const fishnet::geometry::Vec2D<T> & point) noexcept {
    return OGRUniquePtr<OGRPoint>(new OGRPoint(double(point.x),double(point.y)));
}

static fishnet::Option<fishnet::geometry::Ring<fishnet::math::DEFAULT_NUMERIC>> fromOGR(const OGRLinearRing& ogrRing) noexcept {
    std::vector<fishnet::geometry::Vec2D<fishnet::math::DEFAULT_NUMERIC>> pointsInOrder;
    for(const auto & ogrPoint : ogrRing){
        pointsInOrder.push_back(fromOGR(ogrPoint));
    }
    try{
        return fishnet::geometry::Ring<fishnet::math::DEFAULT_NUMERIC>(pointsInOrder);
    }catch(std::invalid_argument & exception){
        spdlog::warn("Invalid ring geometry: {}", exception.what());
        return {};
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

static fishnet::Option<fishnet::geometry::Polygon<fishnet::math::DEFAULT_NUMERIC>> fromOGR(const OGRPolygon & ogrPolygon, bool checked = false) noexcept {
    try{
        auto ogrBoundary = ogrPolygon.getExteriorRing();
        return fromOGR(*ogrBoundary).transform([checked,&ogrPolygon](auto && ring){
            std::vector<fishnet::geometry::Ring<fishnet::math::DEFAULT_NUMERIC>> holes;
            auto inserter = [&holes](auto && hole){
                holes.push_back(std::move(hole));
            };
            for(int i = 0; i < ogrPolygon.getNumInteriorRings(); i++){
                fromOGR(*(ogrPolygon.getInteriorRing(i))).if_value(inserter);
            }
            return fishnet::geometry::Polygon<fishnet::math::DEFAULT_NUMERIC>(ring,holes, checked);
        });
    }catch(fishnet::geometry::InvalidGeometryException & e){
        spdlog::warn("Invalid polygon geometry: {}", e.what());
        return {};
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

static fishnet::Option<fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<fishnet::math::DEFAULT_NUMERIC>>> fromOGR(const OGRMultiPolygon & multiPolygon, bool checked = false) noexcept {
    try{
        
        std::vector<fishnet::geometry::Polygon<fishnet::math::DEFAULT_NUMERIC>> polygons;
        auto inserter = [&polygons](auto && polygon){
            polygons.push_back(std::move(polygon));
        };
        for(auto ogrPolygonPtr : multiPolygon) {
            fromOGR(*ogrPolygonPtr).if_value(inserter);
        }
        return fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<double>>(polygons, checked);
    }catch(fishnet::geometry::InvalidGeometryException & ex) {
        spdlog::warn("Invalid multipolygon geometry: {}", ex.what());
        return {};
    }
}


template<fishnet::geometry::GeometryType G>
constexpr static auto fromOGR(const OGRGeometry & ogrGeometry, bool checked = false){
    if constexpr(G == fishnet::geometry::GeometryType::POLYGON){
        return fromOGR(*ogrGeometry.toPolygon(), checked);
    } else if constexpr(G == fishnet::geometry::GeometryType::POINT){
        return fromOGR(*ogrGeometry.toPoint());
    } else if constexpr(G == fishnet::geometry::GeometryType::RING){
        return fromOGR(*ogrGeometry.toLinearRing());
    }else if constexpr(G == fishnet::geometry::GeometryType::MULTIPOLYGON){
        return fromOGR(*ogrGeometry.toMultiPolygon(), checked);
    }else {
        return std::optional<fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<double>>>();
    }
}

};
}