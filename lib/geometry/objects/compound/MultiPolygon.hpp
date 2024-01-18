#pragma once
#include <ranges>
#include <unordered_set>

#include "FunctionalConcepts.hpp" 

#include "ShapeGeometry.hpp"
#include "InvalidGeometryException.hpp"
#include "Vec2D.hpp"
#include "SimplePolygon.hpp"
#include "PolygonFilter.hpp"

namespace fishnet::geometry{

template<IPolygon P>
class MultiPolygon{
private:
    std::vector<P> polygons;

    constexpr inline bool anyOf(util::Predicate<P> auto && predicate) const noexcept{
        return std::ranges::any_of(polygons,predicate);
    }

    constexpr std::optional<std::string> tryAddGetCauseOnError(const P & polygon) noexcept {
        for(const auto & p : polygons){
            if(p == polygon)
                return "Duplicate polygon: "+p.toString();
            if(p.crosses(polygon))
                return "Polygon: "+polygon.toString()+"\nintersects another polygon:\n"+p.toString();
            if(p.contains(polygon) || polygon.contains(p))
                return "Polygons are contained within each other:\n"+polygon.toString()+"\n and \n"+p.toString();
        }
        polygons.push_back(polygon);
        return {};
    }

    constexpr void addOrThrow(const P & polygon) {
        auto error = tryAddGetCauseOnError(polygon);
        if(error)
            throw InvalidGeometryException(error.value());
    }


public:
    using numeric_type = typename P::numeric_type;
    using polygon_type = P;
    constexpr static GeometryType type = GeometryType::MULTIPOLYGON;


    MultiPolygon(util::input_range_of<P> auto const & polygonsRange,bool checked =false){
        if (checked){
            this->polygons = std::vector<P>(std::ranges::begin(polygonsRange),std::ranges::end(polygonsRange));
        }else{
            if(util::size(polygonsRange) < 10)
                std::ranges::for_each(polygonsRange,[this](const P & p){addOrThrow(p);});
            else {
                this->polygons = filter(polygonsRange,[](const P & lhs, const P & rhs){
                    return lhs == rhs || lhs.crosses(rhs) || lhs.contains(rhs) || rhs.contains(lhs);
                });
            }
        }
    }

    MultiPolygon(const P & polygon):polygons({polygon}) {}

    MultiPolygon(const P & polygon, const P & otherPolygon){
        addOrThrow(polygon);
        addOrThrow(otherPolygon);
    }

    template<typename... Args>
    MultiPolygon(const P & current, const P & next, Args... remaining):MultiPolygon(next,remaining...){
        addOrThrow(current);
    }

    constexpr util::view_of<P> auto getPolygons() const noexcept{
        return std::views::all(polygons);
    }

    template<bool checked=false>
    constexpr bool addPolygon(const P & polygon) noexcept {
        if constexpr(checked){
            this->polygons.push_back(polygon);
            return true;
        }else {
            return not tryAddGetCauseOnError(polygon).has_value();
        }
    }

    constexpr bool removePolygon(const P & polygon) noexcept {
        auto [position,end] = std::ranges::remove(polygons,polygon);
        if(position==end) 
            return false;
        polygons.erase(position,end);
        return true;
    }


    constexpr fishnet::math::DEFAULT_FLOATING_POINT area() const noexcept {
        auto viewOnPolygonArea = std::views::all(polygons) | std::views::transform([](const P & p){return p.area();});
        return std::accumulate(std::ranges::begin(viewOnPolygonArea),std::ranges::end(viewOnPolygonArea),0.0);
    } 

    constexpr Vec2DReal centroid() const noexcept {
        Vec2DReal accumulatedWeightedCentroid {0.0,0.0};
        fishnet::math::DEFAULT_FLOATING_POINT totalArea = this->area();
        for(const auto & polygon: polygons) {
            accumulatedWeightedCentroid = accumulatedWeightedCentroid + polygon.centroid() * (polygon.area() / totalArea);
        }
        return accumulatedWeightedCentroid;
    }

    constexpr bool contains(IPoint auto const & point) const noexcept {
        return anyOf([&point](const P & p){return p.contains(point);});
    }

    constexpr bool contains(ISegment auto const & segment) const noexcept{
        return anyOf([&segment](const P & p){return p.contains(segment);});
    }

    constexpr bool isInside(IPoint auto const & point ) const noexcept {
        return anyOf([&point](const P & p){return p.isInside(point);});
    }

    constexpr bool isOnBoundary(IPoint auto const & point) const noexcept {
        return anyOf([&point](const P & p){return p.isOnBoundary(point);});
    }

    constexpr bool isOutside(IPoint auto const & point) const noexcept {
        return std::ranges::all_of(polygons,[&point](const P & p){return p.isOutside(point);});
    }

    constexpr bool intersects(LinearGeometry auto const & linearGeoemtry) const noexcept {
        return anyOf([&linearGeoemtry](const P & p){return p.intersects(linearGeoemtry);});
    }

    constexpr util::forward_range_of<Vec2DReal> auto intersections(LinearGeometry auto const & linearGeometry) const noexcept {
        std::unordered_set<Vec2DReal> intersectionSet;
        std::ranges::for_each(polygons,[&intersectionSet,&linearGeometry](const P & p){
            std::ranges::for_each(p.intersections(linearGeometry),[&intersectionSet](const auto & point){
                intersectionSet.insert(point);
            });
        });
        return intersectionSet;
    }

    constexpr bool contains(IPolygon auto const & query) const noexcept {
        return anyOf([&query](const P & p){return p.contains(query);});
    }

    constexpr bool crosses(IPolygon auto const & query) const noexcept {
        return anyOf([&query](const P & p){return p.crosses(query);});
    }

    constexpr bool touches(IPolygon auto const & query) const noexcept {
        return not contains(query) && not crosses(query) && anyOf([&query](const P & p){return p.touches(query);});
    }

    constexpr bool isInHole(IPolygon auto const & query) const noexcept  {
        return anyOf([&query](const P & p){return p.isInHole(query);});
    }

    constexpr fishnet::math::DEFAULT_FLOATING_POINT distance(IPolygon auto const & query) const noexcept {
        return std::ranges::min(this->getPolygons() | std::views::transform([&query](const P & p){return p.distance(query);}));
    }

    constexpr fishnet::math::DEFAULT_FLOATING_POINT distance(IMultiPolygon auto const & other) const noexcept {
        return std::ranges::min( other.getPolygons() | std::views::transform([this](const auto & otherPolygon){return this->distance(otherPolygon);}));
    }

    constexpr IRing<numeric_type> auto aaBB() const noexcept {
        auto initialPoint = std::ranges::begin(this->polygons.at(0).getBoundary().getSegments())->p();
        numeric_type high = initialPoint.y;
        numeric_type low = high;
        numeric_type right = initialPoint.x;
        numeric_type left = right;
        auto viewOnAllSegments = this->getPolygons() 
            | std::views::transform([](const P & p){return p.getBoundary().getSegments();}) 
            | std::views::join;
        for(const auto s: viewOnAllSegments){
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
        return SimplePolygon<numeric_type>({{left,high},{right,high},{right,low},{left,low}});
    }

    template<IMultiPolygon M>
    constexpr bool operator==(M const & other) const noexcept {
        if(util::size(this->polygons) != util::size(other.getPolygons()))
            return false;
        for(const auto & p: other.getPolygons()){
            auto foundIt = std::ranges::find(this->polygons,p);
            if(foundIt == std::ranges::end(this->getPolygons()))
                return false;
        }
        return true;
    }

    constexpr std::string toString() const noexcept {
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        for(const auto & p : this->polygons){
            if(!first) oss << "|";
            oss << p.toString();
            first = false;
        }
        oss << "}";
        return oss.str();
    }
};
}
namespace std{
    template<typename P>
    struct hash<fishnet::geometry::MultiPolygon<P>>{
        constexpr static auto polygonHasher = std::hash<P>{};
        size_t operator()(const fishnet::geometry::MultiPolygon<P> & multiPolygon) const noexcept {
            auto polygonHashes = multiPolygon.getPolygons() | std::views::transform([](const auto & p){return polygonHasher(p);});
            return std::accumulate(std::ranges::begin(polygonHashes),std::ranges::end(polygonHashes),0);
        }
    };
}