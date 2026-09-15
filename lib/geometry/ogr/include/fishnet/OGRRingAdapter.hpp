#pragma once
#include <fishnet/IGeometry.hpp>
#include <fishnet/Segment.hpp>
#include <fishnet/Rectangle.hpp>
#include <fishnet/RingIntersection.hpp>
#include <fishnet/PolygonDistance.hpp>
#include "OGRPolygonalAdapter.hpp"

namespace fishnet::geometry{

/**
 * @brief IRing implementation backed by an OGRPolygon holding nothing but an exterior ring
 *
 * The wrapped ring is always kept closed (first point == last point), as OGR/OGC require it.
 * The IRing facing API however exposes the open representation used throughout fishnet:
 * a ring over the points u,v,x,y,z reports exactly those points and the five segments
 * u-v, v-x, x-y, y-z and z-u.
 * @note the points are reported in the canonical order of the wrapped polygon, which is not
 * necessarily the order they were handed in with, @see OGRPolygonalAdapter::canonicalize()
 */
class OGRRingAdapter: public OGRPolygonalAdapter{
private:
    static OGRUniquePtr<OGRPolygon> asPolygon(OGRUniquePtr<OGRLinearRing> && ringPtr) {
        ringPtr->closeRings();
        auto polygon = emptyPolygon();
        polygon->addRingDirectly(ringPtr.release());
        return polygon;
    }

    OGRRingAdapter(OGRUniquePtr<OGRPolygon> && polygonPtr, Canonical canonical)
        :OGRPolygonalAdapter(std::move(polygonPtr), canonical) {}

    /**
     * @brief Adopt a ring which is known to be in canonical form already
     *
     * The exterior ring of a canonical polygon is itself canonical, so extracting it does not have
     * to pay for another GEOS normalisation. This does NOT hold for the interior rings: canonical
     * holes are wound against their shell, so a hole has to be normalised to be comparable with a
     * ring of the same shape built on its own. Hence this is reserved for OGRPolygonAdapter
     * handing out its boundary, and not part of the public interface.
     */
    static OGRRingAdapter fromCanonicalRing(const OGRLinearRing & ring) {
        auto polygon = emptyPolygon();
        polygon->addRingDirectly(ring.clone());
        return OGRRingAdapter(std::move(polygon), Canonical{});
    }

    /**
     * @brief Adopt an interior ring of a canonical polygon
     *
     * Canonical polygons wind their holes against their shell, but the holes are in canonical form
     * apart from that. Flipping the winding therefore yields the canonical standalone ring, which
     * costs a linear pass over the points instead of another GEOS normalisation.
     */
    static OGRRingAdapter fromCanonicalHole(const OGRLinearRing & hole) {
        auto ring = OGRUniquePtr<OGRLinearRing>(hole.clone());
        ring->reverseWindingOrder();
        auto polygon = emptyPolygon();
        polygon->addRingDirectly(ring.release());
        return OGRRingAdapter(std::move(polygon), Canonical{});
    }


    /**
     * @brief Wrap a ring owned by somebody else, without copying its points
     *
     * The ring is put into a shell polygon, which is what this adapter works on, and is handed
     * back to its owner when the shell is destroyed. Only for operating on the rings of a polygon
     * in place, which outlives the operation.
     * @note a borrowed hole is wound against its shell, so it is not in canonical form and must
     * not be compared or hashed; its geometry is unaffected
     * @warning the result must not outlive the owner of the ring
     */
    static OGRRingAdapter borrowed(OGRLinearRing * ring) {
        return OGRRingAdapter(__impl::borrowRingAsPolygon(ring), Canonical{});
    }

    friend class OGRPolygonAdapter;

    /**
     * @brief Number of points of the open representation, i.e. without the closing point
     * @note an empty ring has no closing point to drop
     */
    int openPointCount() const {
        return std::max(0, exteriorRing()->getNumPoints() - 1);
    }

public:
    using OGRPolygonalAdapter::contains;

    using numeric_type = double;
    static constexpr GeometryType type = GeometryType::RING;

    OGRRingAdapter(OGRUniquePtr<OGRLinearRing> && ringPtr):OGRPolygonalAdapter(asPolygon(std::move(ringPtr))) {}

    OGRRingAdapter(OGRUniquePtr<OGRPolygon> && polygonPtr):OGRPolygonalAdapter(std::move(polygonPtr)) {}

    OGRRingAdapter(IRing auto const & ring):OGRPolygonalAdapter(toOGRPolygon(ring)) {}

    auto getSegments() const -> fishnet::util::forward_range_of<fishnet::geometry::Segment<double>> auto {
        return std::ranges::views::iota(0, openPointCount())
            | std::ranges::views::transform([this](int i) {
                const auto * ring = exteriorRing();
                Vec2DReal p1 {ring->getX(i), ring->getY(i)};
                Vec2DReal p2 {ring->getX(i + 1), ring->getY(i + 1)};
                return Segment<double>(p1, p2);
            });
    }

    auto getPoints() const -> fishnet::util::forward_range_of<fishnet::geometry::Vec2DReal> auto {
        return std::ranges::views::iota(0, openPointCount())
            | std::ranges::views::transform([this](int i) {
                const auto * ring = exteriorRing();
                return Vec2DReal(ring->getX(i), ring->getY(i));
            });
    }

    const OGRRingAdapter & getBoundary() const{
        return *this;
    }

    std::vector<OGRRingAdapter> getHoles() const{
        return {};
    }

    /**
     * @brief Centroid of the ring, i.e. the mean of its points
     * @note this is the centroid definition used by fishnet::geometry::Ring, not the area weighted
     * centroid OGR/GEOS would compute. The two only coincide for symmetric rings.
     */
    Vec2DReal centroid() const {
        Vec2DReal sum {0,0};
        size_t count = 0;
        for(const auto & point : getPoints()){
            sum = sum + point;
            ++count;
        }
        return sum / static_cast<double>(count);
    }

    bool intersects(const LinearGeometry auto & linearFeature) const{
        return ringIntersects(*this, linearFeature);
    }

    std::unordered_set<Vec2DReal> intersections(const LinearGeometry auto & linearFeature) const{
        auto intersectionRange = this->getSegments()
            | std::views::transform([&linearFeature](const auto & segment){return segment.intersection(linearFeature);})
            | std::views::filter([](const auto & optIntersection){return optIntersection.has_value();})
            | std::views::transform([](const auto & optIntersection){return optIntersection.value();});
        return std::unordered_set<Vec2D<double>> {std::ranges::begin(intersectionRange), std::ranges::end(intersectionRange)};
    }

    bool crosses(const IRing auto & other) const{
        if(not this->aaBB().overlap(other.aaBB()))
            return false;
        return std::ranges::any_of(this->getSegments(),[&other](const auto & s){return other.intersects(s);})
            || std::ranges::any_of(other.getSegments(),[this](const auto & s){return this->intersects(s);});
    }

    bool touches(const IRing auto & other) const{
        if(not this->aaBB().overlap(other.aaBB()))
            return false;
        if(this->crosses(other)) 
            return false;
        if(this->contains(other) || other.contains(*this)) return false;
        for(const auto & p : other.getPoints()){
            if(this->isOnBoundary(p)) return true;
        }
        return false;
    }

    double distance(const IRing auto & other) const{
        if(this->contains(other) or other.contains(*this) or this->crosses(other))
             return 0;
        return shapeDistance(*this,other);
    }

    double distance(const OGRRingAdapter & other) const{
        if(this->contains(other) or other.contains(*this) or this->crosses(other))
             return 0;
        return geomPtr->Distance(other.geomPtr.get());
    }
};
static_assert(IRing<OGRRingAdapter>);
}
