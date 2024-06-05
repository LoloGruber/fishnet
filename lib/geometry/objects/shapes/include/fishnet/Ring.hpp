#pragma once
#include <vector>
#include <ranges>
#include <stdexcept>
#include <algorithm>
#include <unordered_set>
#include <sstream>

#include <fishnet/Segment.hpp>
#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/Ray.hpp>
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/PolygonalRingVerification.hpp>
#include <fishnet/PolygonDistance.hpp>

namespace fishnet::geometry{

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

    Ring(util::random_access_range_of<Vec2D<T>> auto const& points){
        this->segments = std::move(toSegments(points));
        verifyPolygonalRing<T>(this->segments);
    }

    Ring(util::random_access_range_of<Segment<T>> auto const& segments):segments(segments){
        makeValid();
        verifyPolygonalRing<T>(this->segments);
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
     * @return Ring representing the aaBB
     */
    constexpr Ring<T> aaBB() const noexcept {
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
        return Ring<T>({{left,high},{right,high},{right,low},{left,low}});
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
     * Check every intersection of the linear feature with the segments of the ring:
     * If the endpoints of the intersected segment are on opposite sides of the linear feature -> TRUE
     * If intersection point is vertex of the ring:
     *      s1.p()-----------s1.q() == intersection point == s2.p()----------s2.q()
     *      -> Test if s1.p() and s2.() are on the same side of the linear feature, if not it must be a intersection -> TRUE
     * Additional checks depending on type linear feature:
     * - L == ISegment:
     *      skip if intersection is endpoint of both the segment of the boundary and the linear feature 
     *      otherwise test if any of the endpoints of the linear feature are outside of the ring -> TRUE 
     * - L == IRing:
     *      skip if intersection is the origin of the ray
     *  
     * @tparam L linear feature type
     * @param linearFeature 
     * @return true 
     * @return false 
     */
    template<LinearGeometry L>
    constexpr bool intersects( const L & linearFeature) const noexcept {
        // Helper lambda to check whether two points are on the same side of the linearFeature (or on the line)
        auto onSameSide = [linearFeature](const Vec2D<T> & lhs, const Vec2D<T> & rhs) {
            auto line = linearFeature.toLine();
            if(line.contains(rhs) || line.contains(lhs)){
                return true;
            }
            return line.isLeft(lhs) == line.isLeft(rhs);
        }; 
        for(size_t i = 0; i < segments.size(); ++i){
            auto current = segments[i];
            auto inter = current.intersection(linearFeature);
            if constexpr(ISegment<L>){
                if(inter && (linearFeature.isEndpoint(inter.value()) && current.isEndpoint(inter.value())))
                     continue;
                if(inter && (linearFeature.isEndpoint(inter.value())) && (isOutside(linearFeature.p()) || isOutside(linearFeature.q())))
                    return true;
            }
            if constexpr(IRay<L>){
                if(inter && inter.value() == linearFeature.origin())
                     continue;
            }
            if(inter && current.isEndpoint(inter.value())) { // intersection is vertex of ring
                if(current.p() == inter.value()){
                    if(not onSameSide(current.q(),segments[(i-1)%segments.size()].p()))
                        return true;
                }else{ // inter.value() == current.q()
                    if(not onSameSide(current.p(),segments[(i+1)%segments.size()].q())) 
                        return true;
                }
            }else if(inter){
                if(not onSameSide(current.p(),current.q())) return true;
            }
        }
        return false;

    }

    constexpr util::forward_range_of<Vec2D<double>> auto intersections(LinearGeometry auto const& linearFeature) const noexcept{
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
            [[unlikely]] if (s.containsSegment(segment))
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
        if(splittingPoints.size() == 2){ // segment under test was not split through segments of the ring -> both endpoints inside -> segment inside
            return contains(segment.p()) && contains(segment.q());
        }   

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

    template<fishnet::math::Number U>
    constexpr bool crosses(const Ring<U> & other) const noexcept {
        return std::ranges::any_of(segments,[&other](const auto & s){return other.intersects(s);}) 
            || std::ranges::any_of(other.getSegments(),[this](const auto & s){return this->intersects(s);});
    }

    template<fishnet::math::Number U>
    constexpr bool contains(const Ring<U> & other) const noexcept {
        return std::ranges::all_of(other.getSegments(), [this](const Segment<U> & s){
            return this->contains(s);
        });
    }

    template<fishnet::math::Number U>
    constexpr bool touches(const Ring<U> & other) const noexcept {
        if(this->crosses(other)) return false;
        if(this->contains(other) || other.contains(*this)) return false;
        for(const auto & p : other.getPoints()){
            if(this->isOnBoundary(p)) return true;
        }
        return false;
    }

    template<fishnet::math::Number U>
    constexpr fishnet::math::DEFAULT_FLOATING_POINT distance(const Ring<U> & other) const noexcept {
        if(this->contains(other) or other.contains(*this) or this->crosses(other))
             return -1;
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
static_assert(Shape<Ring<double>>);

//Deduction guides
template<std::ranges::random_access_range R>
Ring(const R &) -> Ring<typename std::ranges::range_value_t<R>::numeric_type>;

template<typename T>
Ring(std::initializer_list<Vec2D<T>> && points)->Ring<T>;

}
namespace std{
    template<typename T>
    struct hash<fishnet::geometry::Ring<T>>{
        constexpr static auto segmentHasher = hash<fishnet::geometry::Segment<T>>{};
        size_t operator()(const fishnet::geometry::Ring<T> & ring) const noexcept{
            auto segmentsHashView = ring.getSegments() | std::views::transform([](const auto & segment){return segmentHasher(segment);});
            return std::accumulate(std::ranges::begin(segmentsHashView),std::ranges::end(segmentsHashView),0);
        }
    };
}