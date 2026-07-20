#pragma once

#include <vector>
#include <unordered_set>
#include <ranges>
#include <algorithm>
#include <memory>

#include <gdal/gdal_alg.h>

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
    explicit DelaunayTriangulation(util::forward_range_of<P> auto const & points) {
        for (const auto & p : points) {
            pointStorage.push_back(p);
        }
        if (pointStorage.size() < 3) {
            triangulation = nullptr;
            return;
        }

        std::vector<double> xs;
        std::vector<double> ys;
        xs.reserve(pointStorage.size());
        ys.reserve(pointStorage.size());
        for (const auto & p : pointStorage) {
            xs.push_back(static_cast<double>(p.x));
            ys.push_back(static_cast<double>(p.y));
        }

        triangulation = GDALTriangulationCreateDelaunay(
            static_cast<int>(pointStorage.size()),
            xs.data(),
            ys.data()
        );
    }

    ~DelaunayTriangulation() {
        if (triangulation) {
            GDALTriangulationFree(triangulation);
        }
    }

    // Move-only (non-copyable because of GDALTriangulation*)
    DelaunayTriangulation(DelaunayTriangulation && other) noexcept
        : triangulation(other.triangulation),
          pointStorage(std::move(other.pointStorage)) {
        other.triangulation = nullptr;
    }

    DelaunayTriangulation & operator=(DelaunayTriangulation && other) noexcept {
        if (this != &other) {
            if (triangulation) GDALTriangulationFree(triangulation);
            triangulation = other.triangulation;
            pointStorage = std::move(other.pointStorage);
            other.triangulation = nullptr;
        }
        return *this;
    }

    DelaunayTriangulation(const DelaunayTriangulation &) = delete;
    DelaunayTriangulation & operator=(const DelaunayTriangulation &) = delete;

    /**
     * @brief Get the triangles as fishnet Triangle objects.
     *
     * Returns an empty Option if the triangulation failed (e.g., fewer than 3 points).
     *
     * @return Option containing vector of Triangle<numeric_type>
     */
    fishnet::Option<std::vector<Triangle<numeric_type>>> getTriangles() const {
        if (!triangulation) return {};
        std::vector<Triangle<numeric_type>> triangles;
        triangles.reserve(triangulation->nFacets);
        for (int i = 0; i < triangulation->nFacets; ++i) {
            const auto & facet = triangulation->pasFacets[i];
            triangles.emplace_back(
                pointStorage[facet.anVertexIdx[0]],
                pointStorage[facet.anVertexIdx[1]],
                pointStorage[facet.anVertexIdx[2]]
            );
        }
        return triangles;
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
    fishnet::Option<std::vector<Segment<numeric_type>>> getEdges() const {
        if (!triangulation) return {};
        std::vector<Segment<numeric_type>> edges;

        std::unordered_set<Segment<numeric_type>> edgeSet;
        edgeSet.reserve(static_cast<size_t>(triangulation->nFacets) * 3);

        for (int i = 0; i < triangulation->nFacets; ++i) {
            const auto & facet = triangulation->pasFacets[i];
            for (int e = 0; e < 3; ++e) {
                int i1 = facet.anVertexIdx[e];
                int i2 = facet.anVertexIdx[(e + 1) % 3];
                edgeSet.emplace(pointStorage[i1], pointStorage[i2]);
            }
        }

        edges.assign(edgeSet.begin(), edgeSet.end());
        return edges;
    }

private:
    GDALTriangulation * triangulation = nullptr;
    std::vector<P> pointStorage;
};

// Deduction guide: infer point type from the range's value type
template<std::ranges::forward_range R>
    requires IPoint<std::ranges::range_value_t<R>>
DelaunayTriangulation(R const &) -> DelaunayTriangulation<std::ranges::range_value_t<R>>;

} // namespace fishnet::geometry
