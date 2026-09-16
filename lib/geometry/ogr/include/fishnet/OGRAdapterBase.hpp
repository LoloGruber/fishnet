#pragma once
#include <gdal/ogr_geometry.h>
#include <memory>
#include <cassert>
#include <fishnet/GeometryBase.hpp>

namespace fishnet::geometry::__impl{

/**
 * @brief What an adapter owns of the geometry it points at
 */
enum class OGROwnership {
    geometry,   ///< the geometry belongs to the adapter and is destroyed with it (the default)
    shellOnly,  ///< only the wrapping polygon belongs to the adapter, its rings are borrowed
    none        ///< the geometry belongs to somebody else
};

/**
 * @brief Deleter of the OGR geometry adapters
 *
 * Owning by default. The other modes let an adapter work on a geometry which belongs to somebody
 * else without copying it. OGR owns its geometries strictly hierarchically - a polygon destroys
 * its rings, a collection its members - so a borrowed geometry has to be released before the
 * wrapper around it is destroyed. That is what this deleter is for, so that no call site has to
 * remember it. @see borrow(), borrowRingAsPolygon()
 */
struct OGRGeometryDeleter {
    OGROwnership ownership = OGROwnership::geometry;

    void operator()(OGRGeometry* geom) const {
        if(not geom || ownership == OGROwnership::none)
            return;
        if(ownership == OGROwnership::shellOnly){
            // hand the borrowed rings back before the shell around them is destroyed, otherwise
            // the shell would take the rings of their actual owner with it
            auto * shell = geom->toPolygon();
            for(int i = shell->getNumInteriorRings(); i >= 0; --i)
                shell->removeRing(i, false);
        }
        OGRGeometryFactory::destroyGeometry(geom);
    }
};
} // namespace fishnet::geometry::__impl

namespace fishnet::geometry{

template<typename T>
using OGRUniquePtr = std::unique_ptr<T, __impl::OGRGeometryDeleter>;

namespace __impl{

/**
 * @brief Point at a geometry which belongs to somebody else, without copying it
 * @warning the result must not outlive that owner, so this is reserved for the internals of the
 * adapters, which can guarantee as much
 */
template<typename T>
static OGRUniquePtr<T> borrow(T * geometry) noexcept {
    return OGRUniquePtr<T>(geometry, OGRGeometryDeleter{OGROwnership::none});
}

/**
 * @brief Wrap a ring which belongs to somebody else into a polygon, without copying its points
 *
 * The adapters operate on an OGRPolygon, as OGR cannot hand a bare OGRLinearRing to GEOS. A ring
 * is therefore put into an otherwise empty polygon, which references the ring rather than copying
 * it and releases it again when destroyed.
 * @warning the result must not outlive the owner of the ring
 */
static OGRUniquePtr<OGRPolygon> borrowRingAsPolygon(OGRLinearRing * ring) noexcept {
    auto * shell = static_cast<OGRPolygon*>(OGRGeometryFactory::createGeometry(wkbPolygon));
    shell->addRingDirectly(ring);
    return OGRUniquePtr<OGRPolygon>(shell, OGRGeometryDeleter{OGROwnership::shellOnly});
}
} // namespace __impl

/**
 * @brief Base class for OGR geometry adapters, which owns an OGR geometry
 * 
 * @tparam T child class of OGRGeometry
 */
template<typename T> requires std::is_base_of_v<OGRGeometry, T>
class OGRAdapterBase{
protected:
    OGRUniquePtr<T> geomPtr;
public:
    using numeric_type = double;

    OGRAdapterBase(OGRUniquePtr<T> && geomPtr):geomPtr(std::move(geomPtr)){
        assert(this->geomPtr != nullptr);
    }

    OGRAdapterBase(OGRAdapterBase && other) noexcept : geomPtr(std::move(other.geomPtr)) {}

    OGRAdapterBase(const OGRAdapterBase & other): geomPtr(OGRUniquePtr<T>(static_cast<T*>(other.geomPtr->clone()))) {}

    OGRAdapterBase& operator=(const OGRAdapterBase & other){
        if(this != &other){
            geomPtr = OGRUniquePtr<T>(static_cast<T*>(other.geomPtr->clone()));
        }
        return *this;
    }
    OGRAdapterBase& operator=(OGRAdapterBase && other) noexcept {
        if(this != &other){
            geomPtr = std::move(other.geomPtr);
        }
        return *this;
    }

    bool operator==(const OGRAdapterBase & other) const noexcept{
        if(this->geomPtr == other.geomPtr) return true; // same pointer, same object
        return geomPtr->Equals(other.geomPtr.get());
    }

    T * raw() const noexcept{
        return geomPtr.get();
    }

    size_t hash() const noexcept {
        size_t nSize = geomPtr->WkbSize();
        std::vector<unsigned char> pabyData(nSize);
        geomPtr->exportToWkb(pabyData.data());
        std::string_view bytes(reinterpret_cast<const char*>(pabyData.data()), nSize);
        return std::hash<std::string_view>{}(bytes);
    }

    std::string toString() const{
        return geomPtr->exportToWkt();
    }
};
/**
 * @brief Convert fishnet::geometry::GeometryType <-> OGRwkbGeometryType
 * 
 */
class GeometryTypeWKBAdapter{
public:
    static OGRwkbGeometryType toWKB(GeometryType type){
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
            // OGR has no WKB type of its own for a linear ring and reports it as a line string
            case wkbLinearRing:
            case wkbLineString: return geometry::GeometryType::RING;
            case wkbPolygon: return geometry::GeometryType::POLYGON;
            case wkbMultiPolygon: return geometry::GeometryType::MULTIPOLYGON;
            default: throw std::invalid_argument("wkbGeometryType could not be converted into a Fishnet geometry type");
        }
    }
};
} // namespace fishnet::geometry
