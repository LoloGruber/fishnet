#pragma once
// NOTE: this header is deliberately not called OGRGeometryAdapter.hpp, although that is the class
// it provides: the legacy fishnet::OGRGeometryAdapter (lib/io/gdal) already occupies that file name
// on the include path. Rename this file once the legacy adapter is retired.
#include <string>
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

    /**
     * @brief WKB type of the wrapped geometry, without its Z/M modifiers
     */
    OGRwkbGeometryType flatType() const noexcept {
        return wkbFlatten(geomPtr->getGeometryType());
    }

public:
    using numeric_type = double;

    OGRGeometryAdapter(OGRUniquePtr<OGRGeometry> && geometryPtr):Base(std::move(geometryPtr)) {}

    // a ring adapter is backed by a polygon, so the exterior ring is what has to be wrapped for
    // the geometry to still be recognised as a ring
    OGRGeometryAdapter(const OGRRingAdapter & ring):Base(OGRUniquePtr<OGRGeometry>(ring.raw()->getExteriorRing()->clone())) {}

    OGRGeometryAdapter(const OGRPolygonAdapter & polygon):Base(OGRUniquePtr<OGRGeometry>(polygon.raw()->clone())) {}

    OGRGeometryAdapter(const OGRMultiPolygonAdapter & multiPolygon):Base(OGRUniquePtr<OGRGeometry>(multiPolygon.raw()->clone())) {}

    /**
     * @brief WKB type of the wrapped geometry, without its Z/M modifiers
     * @note OGR reports wkbLineString for an OGRLinearRing, as a linear ring has no WKB type of
     * its own; @see isRing()
     */
    OGRwkbGeometryType wkbType() const noexcept {
        return flatType();
    }

    std::string geometryName() const noexcept {
        return geomPtr->getGeometryName();
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
        return flatType() == wkbLineString && not isEmpty() && geomPtr->toLineString()->getNumPoints() >= 3;
    }

    bool isPolygon() const noexcept {
        return flatType() == wkbPolygon && not isEmpty();
    }

    bool isMultiPolygon() const noexcept {
        return flatType() == wkbMultiPolygon && not isEmpty();
    }

    /**
     * @brief Convert to an OGRRingAdapter
     * @return empty Option if the wrapped geometry is not a ring
     */
    Option<OGRRingAdapter> toRing() const {
        if(not isRing())
            return {};
        // works for an OGRLinearRing as well as for a plain OGRLineString, and avoids downcasting
        // to OGRLinearRing, which would be undefined behaviour for the latter
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

    /**
     * @note OGRAdapterBase::hash() cannot be used, as a geometry which happens to be a bare
     * OGRLinearRing has no WKB representation
     */
    size_t hash() const noexcept {
        return std::hash<std::string>{}(this->toString());
    }
};
}
