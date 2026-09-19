#pragma once
#include <fishnet/IGeometry.hpp>
#include <fishnet/Segment.hpp>
#include <fishnet/Rectangle.hpp>
#include <fishnet/RingIntersection.hpp>
#include <fishnet/PolygonDistance.hpp>
#include <fishnet/CollectionConcepts.hpp>
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
 * @note getSegments() and getPoints() are ref-qualified: the lvalue (`&`) overload returns a view
 * that lazily reads through `this` on every access, cheap but only valid as long as this adapter is
 * (e.g. a named local or a longer-lived object accessed by reference); the rvalue (`&&`) overload,
 * selected when `this` is itself a temporary, e.g. `polygon.getBoundary().getSegments()` where
 * getBoundary() hands back a fresh, unnamed OGRRingAdapter, instead copies the result out into an
 * owning vector before that temporary is destroyed at the end of the full expression. Splitting on
 * value category like this keeps both the common lazy path and this once-off temporary chain safe
 * without callers having to know or care which one they hit.
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
        ring->reversePoints();
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
    friend class OGRGeometryAdapter;

    /**
     * @brief Number of points of the open representation, i.e. without the closing point
     * @note an empty ring has no closing point to drop
     */
    int openPointCount() const {
        return std::max(0, exteriorRing()->getNumPoints() - 1);
    }

    /**
     * @brief Hand the wrapped ring over as a standalone geometry, without copying its points
     *
     * Detaches the exterior ring from the shell polygon it is kept in rather than cloning it, so
     * the shell is left empty (and is discarded along with this adapter, which must not be used
     * again on success).
     */
    OGRUniquePtr<OGRGeometry> releaseGeometry() && noexcept {
        auto * ring = geomPtr->getExteriorRing();
        geomPtr->removeRing(0, false); // detach without deleting; the emptied shell is discarded with this adapter
        return OGRUniquePtr<OGRGeometry>(ring);
    }

public:
    using OGRPolygonalAdapter::contains;

    using numeric_type = double;
    static constexpr GeometryType type = GeometryType::RING;

    OGRRingAdapter(OGRUniquePtr<OGRLinearRing> && ringPtr):OGRPolygonalAdapter(asPolygon(std::move(ringPtr))) {}

    OGRRingAdapter(OGRUniquePtr<OGRPolygon> && polygonPtr):OGRPolygonalAdapter(std::move(polygonPtr)) {}

    OGRRingAdapter(IRing auto const & ring):OGRPolygonalAdapter(toOGRPolygon(ring)) {}

    /**
     * @brief The segments of the ring, lazily read through this adapter on every access
     * @note lvalue overload: the returned view keeps reading through `this` on every iteration, which
     * is cheap as long as the adapter it reads through outlives the view, e.g. a named local or a
     * longer-lived object accessed by reference. @see the `&&` overload below for the rvalue case.
     */
    auto getSegments() const & -> fishnet::util::forward_range_of<fishnet::geometry::Segment<double>> auto {
        return std::ranges::views::iota(0, openPointCount())
            | std::ranges::views::transform([this](int i) {
                const auto * ring = exteriorRing();
                Vec2DReal p1 {ring->getX(i), ring->getY(i)};
                Vec2DReal p2 {ring->getX(i + 1), ring->getY(i + 1)};
                return Segment<double>(p1, p2);
            });
    }

    /**
     * @brief The segments of the ring, eagerly copied out into an owning vector
     * @note rvalue overload, selected when `this` is about to be destroyed, e.g.
     * `polygon.getBoundary().getSegments()` where getBoundary() hands back a fresh, unnamed
     * OGRRingAdapter: the lvalue overload's view would keep reading through that temporary after it
     * is gone, at the end of the full expression. A `const &` overload alone would not catch this,
     * since a const lvalue reference binds to an rvalue too and the view would dangle just the same;
     * only an overload specifically for rvalues lets us swap in a self-contained copy instead. Callers
     * do not need to know or care which overload fires: both hand back a forward_range of Segment.
     */
    std::vector<fishnet::geometry::Segment<double>> getSegments() const && {
        return fishnet::util::toVector(this->getSegments());
    }

    /// @copydoc getSegments()
    auto getPoints() const & -> fishnet::util::forward_range_of<fishnet::geometry::Vec2DReal> auto {
        return std::ranges::views::iota(0, openPointCount())
            | std::ranges::views::transform([this](int i) {
                const auto * ring = exteriorRing();
                return Vec2DReal(ring->getX(i), ring->getY(i));
            });
    }

    /// @copydoc getSegments() const&&
    std::vector<fishnet::geometry::Vec2DReal> getPoints() const && {
        return fishnet::util::toVector(this->getPoints());
    }

    const OGRRingAdapter & getBoundary() const{
        return *this;
    }

    /**
     * @brief Copy the ring out into a fishnet Ring, which precomputes its segments
     *
     * Segment access on an adapter is lazy, so code walking the same ring over and over is better
     * served by a native ring. The points come from OGR and are taken as valid, validating them
     * again here would cost far more than the conversion itself.
     */
    Ring<double> toNative() const {
        return Ring<double>(getPoints(), true);
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
        return geomPtr->Distance(other.geomPtr.get());
    }
};
static_assert(IRing<OGRRingAdapter>);
}
