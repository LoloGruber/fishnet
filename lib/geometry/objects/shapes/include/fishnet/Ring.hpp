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

namespace fishnet::geometry{

enum class PointLocation{
    INSIDE,OUTSIDE,BOUNDARY
};



template<fishnet::math::Number T>
class Ring{
private:
    std::vector<Segment<T>> segments;

    constexpr static inline std::vector<Segment<T>> toSegments(util::random_access_range_of<Vec2D<T>> auto const & points) noexcept {
        std::vector<Segment<T>> s {};
        if(points.size()==0) return s;
        s.reserve(points.size());
        for(size_t i = 0; i < points.size()-1 ; ++i){
            if(points[i]==points[i+1]) continue;
            s.emplace_back(Segment(points[i],points[i+1]));
        }
        auto const& first = points[0];
        auto const& last = points[points.size()-1];
        if(first != last){
            s.emplace_back(Segment(last,first));
        }
        return s;
    }

    constexpr void makeValid() noexcept {
        for(size_t i =0; i < segments.size(); ++i){
            if(segments[i].q() != segments[(i+1)%segments.size()].p()){
                auto & s = segments[(i+1)%segments.size()];
                s = s.flip();
            }
        }
    }
protected:

    constexpr PointLocation getPointLocation(IPoint auto const & point) const noexcept {
        u_int16_t intersectionCounter = 0;
        Ray<T> horizontalRay = Ray<T>::right(point);
        for(const auto & segment: segments){
            if(point==segment.p() or point==segment.q() or segment.contains(point)) return PointLocation::BOUNDARY;
            std::optional<Vec2DReal> interOpt = segment.intersection(horizontalRay);
            if (not interOpt) continue;
            Vec2DReal const & inter = interOpt.value();
            if (inter == segment.lowerEndpoint()) continue; //prevent counting a vertex twice -> count only upperEndpooints
            ++intersectionCounter; 
        }
        return intersectionCounter%2 == 1 ? PointLocation::INSIDE : PointLocation::OUTSIDE;
    }

    constexpr static inline fishnet::math::DEFAULT_FLOATING_POINT minDistanceChecked(IRing auto const & thisRing,IRing auto const & other ) noexcept{
        fishnet::math::DEFAULT_FLOATING_POINT minDistance = std::numeric_limits<fishnet::math::DEFAULT_FLOATING_POINT>::max();
        for(const auto & s : thisRing.getSegments()){
            for(const auto & p : other.getPoints()){
                if(s.distance(p) < minDistance){
                    minDistance = s.distance(p);
                }
            }
        }
        for(const auto & s : other.getSegments()){
            for(const auto & p: thisRing.getPoints()){
                if(s.distance(p) < minDistance){
                    minDistance = s.distance(p);
                }
            }
        }
        return minDistance;
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

    constexpr util::view_of<Segment<T>> auto getSegments() const noexcept{
        return std::views::all(segments);
    }

    constexpr util::view_of<Vec2D<T>> auto getPoints() const noexcept {
        return getSegments() 
            | std::views::transform([](const auto & s){return s.p();});
    }

    constexpr fishnet::math::DEFAULT_FLOATING_POINT area() const noexcept {
        fishnet::math::DEFAULT_FLOATING_POINT area = 0;
        auto const& points = this->getPoints();
        for(size_t i = 0; i < points.size(); i++){
            area += points[i].cross(points[(i+1)%points.size()]);
        }
        return 0.5 * fabs(area);
    }

    //https://en.wikipedia.org/wiki/Centroid of finite set of points
    constexpr Vec2DReal centroid() const noexcept {
        Vec2DReal sum {0,0};
        for(const auto & s : segments){
            sum = sum + s.p();
        }
        return sum / (fishnet::math::DEFAULT_FLOATING_POINT)segments.size();
    }

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

    template<LinearGeometry L>
    constexpr bool intersects( const L & linearFeature) const noexcept {
        auto onSameSide = [linearFeature](const Vec2D<T> & lhs, const Vec2D<T> & rhs) {
            auto line = linearFeature.toLine();
            if(line.contains(rhs) || line.contains(lhs)){
                return true;
            }
            return line.isLeft(lhs) == line.isLeft(rhs);
        }; 
        // todo clean up mess
        for(size_t i = 0; i < segments.size(); ++i){
            auto current = segments[i];
            auto inter = current.intersection(linearFeature);
            if constexpr(std::same_as<L,Segment<typename L::numeric_type>>){
                if(inter && (linearFeature.isEndpoint(inter.value()) && current.isEndpoint(inter.value())))
                     continue;
                if(inter && (linearFeature.isEndpoint(inter.value())) && (isOutside(linearFeature.p()) || isOutside(linearFeature.q())))
                    return true;
            }
            if constexpr(std::same_as<L,Ray<typename L::numeric_type>>){
                if(inter && inter.value() == linearFeature.origin()) continue;
            }
            if(inter && current.isEndpoint(inter.value())) { // is vertex of ring
                if(current.p() == inter.value()){
                    if(not onSameSide(current.q(),segments[(i-1)%segments.size()].p())) return true;
                }else{
                    if(not onSameSide(current.p(),segments[(i+1)%segments.size()].q())) return true;
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

    constexpr bool contains(ISegment auto const & segment) const noexcept {
        [[unlikely]] if(not segment.isValid()) return contains(segment.p());
        std::vector<Vec2DReal> splittingPoints;
        splittingPoints.push_back(segment.p());
        for(const auto & s : segments){
            [[unlikely]] if (s.containsSegment(segment)) return true;
            auto inter = s.intersection(segment);
            if(inter and not s.isEndpoint(*inter) and not segment.isEndpoint(*inter)){
                splittingPoints.push_back(*inter);
            }
        }
        splittingPoints.push_back(segment.q());
        std::ranges::sort(splittingPoints,[segment](const Vec2DReal & a, const Vec2DReal & b ){
            return segment.p().distance(a) < segment.p().distance(b);
        });
        for(size_t i = 0; i < splittingPoints.size()-1; i++){
            auto middlePointOfPartialSegment = splittingPoints[i] + (splittingPoints[i+1]-splittingPoints[i]) * 0.5;
            if(not contains(middlePointOfPartialSegment)) return false;
        }   
        return true;
    }


    template<fishnet::math::Number U>
    constexpr bool operator==(const Ring<U> & other) const noexcept {
        if(this->segments.size() != other.getSegments().size()) return false;
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
        if(indexOfStart == -1) return false;

        auto nextIndex = [size,indexOfStart](size_t index){return (indexOfStart+index) % size;};
        bool allMatch = true;
        for(size_t i = 0; i < this->segments.size(); ++i){
            if(this->segments[i] != segmentViewOther[nextIndex(i)]) allMatch = false;
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
        //TODO move all (complex or quadratic) algorithms in "geometry_algo" with the option of implementing faster versions
        if(this->contains(other) or other.contains(*this) or this->crosses(other)) return -1;
        return minDistanceChecked(*this,other);

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
static_assert(ShapeGeometry<Ring<double>>);

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