#pragma once

#include <vector>
#include <unordered_set>
#include <ranges>
#include <algorithm>
#include <memory>

#include <ogr_geometry.h>

#include <fishnet/Vec2D.hpp>
#include <fishnet/LinearGeometry.hpp>
#include <fishnet/Segment.hpp>
#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/Triangle.hpp>
#include <fishnet/Option.hpp>

namespace fishnet::geometry {

/**
 * @brief Wrapper around GDAL's Delaunay triangulation (GDALTriangulationCreateDelaunay).
 *
 * Takes a range of IPoint-conforming points, feeds them to GDAL, and exposes the result
 * as fishnet Triangle objects and unique Segment edges.
 *
 * @tparam P point type conforming to IPoint
 */
template<IPoint P>
class DelaunayTriangulation {
public:
    using point_type = P;
    using numeric_type = typename P::numeric_type;

    /**
     * @brief Construct from a forward range of points.
     * @param points range of points to triangulate
     */
    explicit DelaunayTriangulation(util::forward_range_of<P> auto const & points, double tolerance = 0.0, bool onlyEdges = false) {
        OGRMultiPoint multipoint = OGRMultiPoint();
        for (const auto & p : points) {
            multipoint.addGeometry(std::make_unique<OGRPoint>(p.x, p.y));
        }
        std::unique_ptr<OGRGeometry> delaunay = std::unique_ptr<OGRGeometry>(multipoint.DelaunayTriangulation(tolerance,onlyEdges));
        if (!delaunay || delaunay->IsEmpty())
            return;
        if (onlyEdges) {
            std::unordered_set<Segment<numeric_type>> delaunayEdges;
            for(const auto & line : delaunay->toMultiLineString()) {
                delaunayEdges.emplace(Vec2DReal{line->getX(0), line->getY(0)}, Vec2DReal{line->getX(1), line->getY(1)});
            }
            _edges = std::move(delaunayEdges);
            return;
        } 
        std::vector<Triangle<numeric_type>> delaunayTriangles;
        delaunayTriangles.reserve(delaunay->toGeometryCollection()->getNumGeometries());
        for(const auto & triangle : delaunay->toGeometryCollection()){
            OGRLinearRing * exteriorRing = triangle->toPolygon()->getExteriorRing();
            fishnet::geometry::Triangle<numeric_type> t(
                {exteriorRing->getX(0), exteriorRing->getY(0)},
                {exteriorRing->getX(1), exteriorRing->getY(1)},
                {exteriorRing->getX(2), exteriorRing->getY(2)}
            );
            delaunayTriangles.push_back(t);
        }
        _triangles = std::move(delaunayTriangles);
        _edges = _triangles.transform([](const auto & triangles){
            return fishnet::util::toUnorderedSet( triangles
                | std::views::transform([](const auto & triangle){return triangle.getSegments();})
                | std::views::join); 
        });   
    }

    /**
     * @brief Get the unique undirected edges of the triangulation.
     *
     * Each edge is returned as a Segment.  Deduplication is performed so
     * every edge appears exactly once.  Segment's commutative operator==
     * and hash make this trivial.
     *
     * Returns an empty Option if the triangulation failed.
     *
     * @return Option containing vector of unique edges as Segments
     */
    fishnet::Option<std::unordered_set<Segment<numeric_type>>> edges() const noexcept {
        return _edges;
    }

    fishnet::Option<std::vector<Triangle<numeric_type>>> triangles() const noexcept{
        return _triangles;
    }

private:
    fishnet::Option<std::unordered_set<Segment<numeric_type>>> _edges;
    fishnet::Option<std::vector<Triangle<numeric_type>>> _triangles;
};

// Deduction guide: infer point type from the range's value type
template<std::ranges::forward_range R>
    requires IPoint<std::ranges::range_value_t<R>>
DelaunayTriangulation(R const &) -> DelaunayTriangulation<std::ranges::range_value_t<R>>;

template<std::ranges::forward_range R>
    requires IPoint<std::ranges::range_value_t<R>>
DelaunayTriangulation(R const &,double,bool) -> DelaunayTriangulation<std::ranges::range_value_t<R>>;

} // namespace fishnet::geometry
