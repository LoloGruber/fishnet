#pragma once
#include <fishnet/Option.hpp>
#include "OGRAdapterBase.hpp"
#include "OGRRingAdapter.hpp"
#include "OGRPolygonAdapter.hpp"
#include "OGRMultiPolygonAdapter.hpp"

namespace fishnet::geometry{

/**
 * @brief Adapter for an OGRGeometry of statically unknown type, as handed out by a data source
 *
 * Narrows the wrapped geometry to one of the concrete adapters. Every conversion verifies the WKB
 * type first and yields an empty Option if the geometry is of another type (or empty), so that a
 * geometry read from a data source can be dispatched without an unchecked downcast:
 *
 *      OGRGeometryAdapter geometry {std::move(geometryPtr)};
 *      geometry.toPolygon().if_value([](const auto & polygon){ ... });
 *
 * The conversions copy the wrapped geometry, leaving this adapter intact.
 */
class OGRGeometryAdapter: public OGRAdapterBase<OGRGeometry>{
private:
    using Base = OGRAdapterBase<OGRGeometry>;
public:
    using numeric_type = double;

    OGRGeometryAdapter(OGRUniquePtr<OGRGeometry> && geometryPtr):Base(std::move(geometryPtr)) {}

    OGRGeometryAdapter(const OGRRingAdapter & ring):Base(OGRUniquePtr<OGRGeometry>(ring.raw()->getExteriorRing()->clone())) {}

    OGRGeometryAdapter(const OGRPolygonAdapter & polygon):Base(OGRUniquePtr<OGRGeometry>(polygon.raw()->clone())) {}

    OGRGeometryAdapter(const OGRMultiPolygonAdapter & multiPolygon):Base(OGRUniquePtr<OGRGeometry>(multiPolygon.raw()->clone())) {}

    /**
     * @brief Geometry type of the wrapped geometry
     * @note OGR reports wkbLineString for an OGRLinearRing, as a linear ring has no WKB type of
     * its own; @see isRing()
     */
    GeometryType geometryType() const noexcept {
        return GeometryTypeWKBAdapter::fromWKB(wkbFlatten(geomPtr->getGeometryType()));
    }

    bool isEmpty() const noexcept {
        return geomPtr->IsEmpty();
    }

    /**
     * @brief Whether the wrapped geometry can be viewed as a ring
     * @note a linear ring is reported as wkbLineString by OGR, so a line string with enough points
     * to enclose an area is accepted here; it is closed on conversion, just like every other ring
     * handed to OGRRingAdapter
     */
    bool isRing() const noexcept {
        return geometryType() == GeometryType::RING && not isEmpty() && geomPtr->toLineString()->getNumPoints() >= 3;
    }

    bool isPolygon() const noexcept {
        return geometryType() == GeometryType::POLYGON && not isEmpty();
    }

    bool isMultiPolygon() const noexcept {
        return geometryType() == GeometryType::MULTIPOLYGON && not isEmpty();
    }

    /**
     * @brief Convert to an OGRRingAdapter
     * @return empty Option if the wrapped geometry is not a ring
     */
    Option<OGRRingAdapter> toRing() const {
        if(not isRing())
            return {};
        const auto * lineString = geomPtr->toLineString();
        auto ring = OGRUniquePtr<OGRLinearRing>(static_cast<OGRLinearRing*>(OGRGeometryFactory::createGeometry(wkbLinearRing)));
        for(int i = 0; i < lineString->getNumPoints(); ++i){
            ring->addPoint(lineString->getX(i), lineString->getY(i));
        }
        return OGRRingAdapter(std::move(ring));
    }

    /**
     * @brief Convert to an OGRPolygonAdapter
     * @return empty Option if the wrapped geometry is not a polygon
     */
    Option<OGRPolygonAdapter> toPolygon() const {
        if(not isPolygon())
            return {};
        return OGRPolygonAdapter(OGRUniquePtr<OGRPolygon>(geomPtr->toPolygon()->clone()));
    }

    /**
     * @brief Convert to an OGRMultiPolygonAdapter
     * @return empty Option if the wrapped geometry is not a multi-polygon
     */
    Option<OGRMultiPolygonAdapter> toMultiPolygon() const {
        if(not isMultiPolygon())
            return {};
        return OGRMultiPolygonAdapter(OGRUniquePtr<OGRMultiPolygon>(geomPtr->toMultiPolygon()->clone()));
    }
};
}
