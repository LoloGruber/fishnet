#pragma once
#include <vector>
#include <algorithm>
#include <optional>
#include <fishnet/SimplePolygon.hpp>
#include <fishnet/Polygon.hpp>
#include <fishnet/MultiPolygon.hpp>
#include <fishnet/PolygonDistance.hpp>

/**
 * @brief Creates an edge (as a SimplePolygon) between the two polygons
 * Widens the segment between the two closest points
 * @param from IPolygon
 * @param to IPolygon
 * @return std::optional<fishnet::geometry::SimplePolygon<double>> contains a simple polygon when edge can be created successfully
 */
static std::optional<fishnet::geometry::SimplePolygon<double>> visualizeEdge(const fishnet::geometry::IPolygon auto & from, const fishnet::geometry::IPolygon auto & to) noexcept{
    auto [l,r] = fishnet::geometry::closestPoints(from,to);
    fishnet::geometry::Segment<double> best {l,r};
    if(not best.isValid())
        return std::nullopt; // dont create edge when polygons touch each other (0-length segment)
    double WIDTH = 0.000001;
    auto orthogonalToSegment = best.direction().orthogonal().normalize(); // orthogonal direction vector 
    auto center = best.p() + best.direction().normalize() * (best.length() / 2); // middle point of the segment
    auto p1 = (best.p() + (orthogonalToSegment * WIDTH / 2));
    auto p2 = (best.p() - (orthogonalToSegment * WIDTH / 2));
    auto p3 = (best.q() + (orthogonalToSegment * WIDTH / 2));
    auto p4 = (best.q() - (orthogonalToSegment * WIDTH / 2));

    /*Comparator to sort the points clockwise*/
    auto cmpClockwise = [&center](fishnet::geometry::Vec2DReal const& u, fishnet::geometry::Vec2DReal const& w) {
        return u.angle(center).getAngleValue() > w.angle(center).getAngleValue();
    };
    std::vector<fishnet::geometry::Vec2D<double>> vectors = {p1, p2, p3, p4};
    std::sort(vectors.begin(), vectors.end(), cmpClockwise);
    try{
        return std::make_optional<fishnet::geometry::SimplePolygon<double>>(vectors);
    }catch(fishnet::geometry::InvalidGeometryException & exc){
        return std::nullopt;
    }
}

/**
 * @brief Creates an edge between the multi-polygons, by creating an edge between the biggest polygon of each multi-polygon
 * 
 * @param from 
 * @param to 
 * @return std::optional<fishnet::geometry::SimplePolygon<double>>: contains a simple polygon when edge can be created successfully
 */
static std::optional<fishnet::geometry::SimplePolygon<double>> visualizeEdge(const fishnet::geometry::IMultiPolygon auto & from, const fishnet::geometry::IMultiPolygon auto & to) noexcept{
    auto areaComparator = [](const auto & lhs, const auto & rhs){return lhs.area() < rhs.area();};
    auto fromBiggest = std::ranges::max_element(from.getPolygons(),areaComparator);
    auto toBiggest = std::ranges::max_element(to.getPolygons(),areaComparator);
    return visualizeEdge(*fromBiggest,*toBiggest);
}