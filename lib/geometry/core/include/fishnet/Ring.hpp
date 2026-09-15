#pragma once
#include <ranges>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <vector>
#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/IGeometry.hpp>
#include <fishnet/Ray.hpp>
#include <fishnet/Segment.hpp>
#include <fishnet/PolygonalRingVerification.hpp>
#include <fishnet/PolygonDistance.hpp>
#include <fishnet/RingIntersection.hpp>

namespace fishnet::geometry{

template<fishnet::math::Number T>
class Rectangle;

enum class PointLocation{
    INSIDE,OUTSIDE,BOUNDARY
};

/**
 * @brief Implementation of a ring
 * 
 * @tparam T numeric type used for computations
 */
template<fishnet::math::Number T>
class Ring{
private:
    std::vector<Segment<T>> segments;

    /**
     * @brief Helper function to create a list of segment from a list of points
     * 
     * @param points range of points
     * @return list of segments formed by the sequence of points
     */
    constexpr static inline std::vector<Segment<T>> toSegments(util::random_access_range_of<Vec2D<T>> auto const & points) noexcept {
        std::vector<Segment<T>> segments {};
        if(points.size()==0) 
            return segments;
        segments.reserve(points.size());
        for(size_t i = 0; i < points.size()-1 ; ++i){
            if(points[i]==points[i+1])
                 continue; // skip 0-length segments
            segments.emplace_back(points[i],points[i+1]);
        }
        auto const& first = points[0];
        auto const& last = points[points.size()-1];
        if(first != last){
            segments.emplace_back(last,first);
        }
        return segments;
    }

    /**
     * @brief Helper method to flip the segments accordingly, such that:
     * Endpoint q() of the current segment == Endpoint p() of the next segment
     */
    constexpr void makeValid() noexcept {
        for(size_t i =0; i < segments.size(); ++i){
            if(segments[i].q() != segments[(i+1)%segments.size()].p()){
                auto & s = segments[(i+1)%segments.size()];
                s = s.flip();
            }
        }
    }
protected:

    /**
     * @brief Get the location of a point with regard to the ring (INSIDE | OUTSIDE | BOUNDARY)
     * Uses the Ray-Casting algorithm:
     * https://en.wikipedia.org/wiki/Point_in_polygon
     * @param point 
     * @return constexpr PointLocation 
     */
    constexpr PointLocation getPointLocation(IPoint auto const & point) const noexcept {
        u_int16_t intersectionCounter = 0;
        Ray<T> horizontalRay = Ray<T>::right(point);
        for(const auto & segment: segments){
            if(point==segment.p() or point==segment.q() or segment.contains(point)) //point is part of any segment on the boundary
                 return PointLocation::BOUNDARY;
            std::optional<Vec2DReal> interOpt = segment.intersection(horizontalRay);
            if (not interOpt) 
                continue; // no intersection, go to next segment
            const Vec2DReal & inter = interOpt.value();
            if (inter == segment.lowerEndpoint()) 
                continue; //prevent counting a vertex twice -> count only upperEndpoints
            ++intersectionCounter; 
        }
        return intersectionCounter%2 == 1 ? PointLocation::INSIDE : PointLocation::OUTSIDE;
    }

public:
    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::RING;

    Ring(util::random_access_range_of<Vec2D<T>> auto const& points, bool checked = false):segments(toSegments(points)){
        if(not checked){
            verifyPolygonalRing<T>(this->segments);
        }
    }

    Ring(util::random_access_range_of<Segment<T>> auto && segments, bool checked = false):segments(std::forward<decltype(segments)>(segments)){
        makeValid();
        if(not checked){
            verifyPolygonalRing<T>(this->segments);
        }
    }

    Ring(std::initializer_list<Vec2D<T>> && points){
        std::vector<Vec2D<T>> pointsInVector {points};
        this->segments = std::move(toSegments(pointsInVector));
        verifyPolygonalRing<T>(this->segments);
    }

    template<fishnet::math::Number U>
    constexpr operator Ring<U> () const noexcept {
        std::vector<Vec2D<U>> points {};
        std::ranges::transform(getPoints(),std::back_inserter(points),[](const auto & p){
            return static_cast<Vec2D<U>>(p);
        });
        return Ring<U>(points);
    }

    constexpr const Ring<T> & getBoundary() const noexcept {
        return *this;
    }

    constexpr auto getHoles() const noexcept {
        return std::ranges::empty_view<Ring<T>>();
    }

    constexpr util::view_of<Segment<T>> auto getSegments() const noexcept{
        return std::views::all(segments);
    }

    constexpr util::view_of<Vec2D<T>> auto getPoints() const noexcept {
        return getSegments() 
            | std::views::transform([](const auto & s){return s.p();});
    }

    /**
     * @brief Calculate the area of the ring
     * Uses Shoelace formula: https://en.wikipedia.org/wiki/Shoelace_formula
     * @return area of the ring in the same units as the segments/points
     */
    constexpr fishnet::math::DEFAULT_FLOATING_POINT area() const noexcept {
        fishnet::math::DEFAULT_FLOATING_POINT area = 0;
        auto const& points = this->getPoints();
        for(size_t i = 0; i < points.size(); i++){
            area += points[i].cross(points[(i+1)%points.size()]);
        }
        return 0.5 * fabs(area);
    }

    /**
     * @brief Calculate centroid point of the ring
     * https://en.wikipedia.org/wiki/Centroid 
     * @return constexpr Vec2DReal 
     */
    constexpr Vec2DReal centroid() const noexcept {
        Vec2DReal sum {0,0};
        for(const auto & s : segments){
            sum = sum + s.p();
        }
        return sum / (fishnet::math::DEFAULT_FLOATING_POINT)segments.size();
    }

    /**
     * @brief Computes the axis-aligned bounding box of the ring
     * Calculated be computing the extreme points in every direction and forming a rectangle
     * @return Rectangle representing the aaBB
     */
    constexpr Rectangle<T> aaBB() const noexcept {
        T high = this->segments.at(0).p().y;
        T low = high;
        T right = this->segments.at(0).p().x;
        T left = right;
        for(const auto s : this->getSegments()){
            auto p = s.p();
            if(p.y > high)
                high = p.y;
            if(p.y < low)
                low = p.y;
            if(p.x > right)
                right = p.x;
            if(p.x < left)
                left = p.x;
        }
        return Rectangle<T>(left,high,right,low);
    }

    constexpr bool contains(IPoint auto const & point) const noexcept {
        return getPointLocation(point) != PointLocation::OUTSIDE;
    }
        
    constexpr bool isInside(IPoint auto const & point) const noexcept {
        return getPointLocation(point) == PointLocation::INSIDE;
    }

    constexpr bool isOnBoundary(IPoint auto const & point) const noexcept {
        return getPointLocation(point) == PointLocation::BOUNDARY;
    }

    constexpr bool isOutside(IPoint auto const & point) const noexcept {
        return getPointLocation(point) == PointLocation::OUTSIDE;
    }

    /**
     * @brief test whether a linear feature intersects the ring
     * @see ringIntersects, which is shared by all IRing implementations
     * @tparam L linear feature type
     * @param linearFeature
     * @return true
     * @return false
     */
    template<LinearGeometry L>
    constexpr bool intersects( const L & linearFeature) const noexcept {
        return ringIntersects(*this, linearFeature);
    }

    constexpr std::unordered_set<Vec2DReal> intersections(LinearGeometry auto const& linearFeature) const noexcept{
         std::unordered_set<Vec2D<double>> intersectionSet {};
        auto intersectionView = this->getSegments() 
            | std::views::transform([linearFeature](const auto & segment){return segment.intersection(linearFeature);})  
            | std::views::filter([](const auto & optIntersection){return optIntersection.has_value();});
        for(auto && optInter : intersectionView){
            intersectionSet.insert(optInter.value());
        }
        return intersectionSet;
    }

    /**
     * @brief Test whether a segment is fully contained inside the boundary of the ring
     * Collect all intersections of the segment with the boundary segments, test if all middle points in-between the intersection points on the segment are contained in the ring
     * @param segment 
     * @return true 
     * @return false 
     */
    constexpr bool contains(ISegment auto const & segment) const noexcept {
        [[unlikely]] if(not segment.isValid())
             return contains(segment.p()); // or segment.q()
        std::vector<Vec2DReal> splittingPoints;
        splittingPoints.push_back(segment.p());
        for(const auto & s : segments){
            [[unlikely]] if (s.contains(segment))
                 return true;
            auto inter = s.intersection(segment);
            if(inter and not s.isEndpoint(inter.value()) and not segment.isEndpoint(inter.value())){ // splitting points are must not be vertices of the ring or endpoint of the segment
                splittingPoints.push_back(inter.value());
            }
        }
        splittingPoints.push_back(segment.q()); // insert at least on splitting point, q() beeing the opposite endpoint of the segment
        std::ranges::sort(splittingPoints,[segment](const Vec2DReal & a, const Vec2DReal & b ){
            return segment.p().distance(a) < segment.p().distance(b); // sort endpoints according to distance from p()
        });
        for(size_t i = 0; i < splittingPoints.size()-1; i++){
            auto middlePointOfPartialSegment = splittingPoints[i] + (splittingPoints[i+1]-splittingPoints[i]) * 0.5;
            if(not contains(middlePointOfPartialSegment)) 
                return false;
        }   
        return true;
    }


    template<fishnet::math::Number U>
    constexpr bool operator==(const Ring<U> & other) const noexcept {
        if(this->segments.size() != other.getSegments().size())
             return false;
        size_t size = segments.size();
        // Find common segment to start comparision
        Segment<T> const & start = this->segments.front();
        auto segmentViewOther = other.getSegments();
        int indexOfStart = -1;
        for(size_t i = 0; i < segmentViewOther.size(); ++i){
            if(segmentViewOther[i] == start){
                indexOfStart = int(i);
                break;
            }
        }
        if(indexOfStart == -1) 
            return false; // no common segment found -> not equal

        auto nextIndex = [size,indexOfStart](size_t index){return (indexOfStart+index) % size;};
        bool allMatch = true;
        for(size_t i = 0; i < this->segments.size(); ++i){
            if(this->segments[i] != segmentViewOther[nextIndex(i)])
                 allMatch = false;
        }
        if(allMatch) return true;
        // Opposite direction
        auto prevIndex = [size,indexOfStart](size_t index) {
            return (indexOfStart-index+size) % size;
        };
        for(size_t i = 0; i < this->segments.size(); ++i){
            if(this->segments[i]!=segmentViewOther[prevIndex(i)]) return false;
        }
        return true;
    }

    constexpr bool crosses(IRing auto const & other) const noexcept {
        if(not this->aaBB().overlap(other.aaBB()))
            return false; // rings whose bounding boxes are apart cannot share a single point
        return std::ranges::any_of(segments,[&other](const auto & s){return other.intersects(s);})
            || std::ranges::any_of(other.getSegments(),[this](const auto & s){return this->intersects(s);});
    }

    constexpr bool contains(IRing auto const & other) const noexcept {
        return std::ranges::all_of(other.getSegments(), [this](const auto & s){
            return this->contains(s);
        });
    }

    constexpr bool touches(IRing auto const & other) const noexcept {
        if(not this->aaBB().overlap(other.aaBB()))
            return false; // rings whose bounding boxes are apart cannot touch
        if(this->crosses(other)) return false;
        if(this->contains(other) || other.contains(*this)) return false;
        for(const auto & p : other.getPoints()){
            if(this->isOnBoundary(p)) return true;
        }
        return false;
    }

    /**
     * @brief Distance between the two rings
     * @return 0 if the rings overlap in any way, i.e. if one contains the other or they cross,
     * otherwise the distance between their boundaries
     */
    constexpr fishnet::math::DEFAULT_FLOATING_POINT distance(IRing auto const & other) const noexcept {
        if(this->aaBB().overlap(other.aaBB()) && (this->contains(other) or other.contains(*this) or this->crosses(other)))
             return 0;
        return shapeDistance(*this,other);
    }

    constexpr std::string toString() const noexcept {
        std::ostringstream oss;
        bool first = true;
        for (const auto & s: this->segments){
            if(!first) oss << ",";
            oss << s.toString();
            first = false;
        }
        return oss.str();
    }
};

//Deduction guides
template<std::ranges::random_access_range R>
Ring(const R &) -> Ring<typename std::ranges::range_value_t<R>::numeric_type>;

template<typename T>
Ring(std::initializer_list<Vec2D<T>> && points)->Ring<T>;

} // namespace fishnet::geometry

namespace std{
    template<typename T>
    struct hash<fishnet::geometry::Ring<T>>{
        constexpr static auto segmentHasher = hash<fishnet::geometry::Segment<T>>{};
        constexpr static auto pointHasher = hash<fishnet::geometry::Vec2DReal>{};
        size_t operator()(const fishnet::geometry::Ring<T> & ring) const noexcept{
            auto segmentsHashView = ring.getSegments() | std::views::transform([](const auto & segment){return segmentHasher(segment);});
            auto centroidHash = pointHasher(ring.centroid());
            return std::accumulate(std::ranges::begin(segmentsHashView),std::ranges::end(segmentsHashView),centroidHash);
        }
    };
}

namespace fishnet::geometry{
// NOTE: static_assert(IRing<Ring<double>>) lives in Rectangle.hpp: the concept can only be
// checked once Rectangle, the envelope type aaBB() returns, is complete.
// Explicit template instantiation
template class Ring<fishnet::math::DEFAULT_NUMERIC>;
} // namespace fishnet::geometry

#include "Rectangle.hpp"
