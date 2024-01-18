#pragma once
#include <set>
#include <ranges>
#include "Vec2D.hpp"
#include "CollectionConcepts.hpp"
#include "FunctionalConcepts.hpp"

namespace fishnet::geometry {

namespace __impl{

template<math::Number T>
constexpr Vec2D<T> nearestNeighbourBinarySearch(const Vec2D<T> & query, const std::set<Vec2D<T>,LexicographicOrder> & points, util::Predicate<Vec2D<T>> auto const & predicate = util::TruePredicate{}) noexcept {
    fishnet::math::DEFAULT_FLOATING_POINT minDistance = std::numeric_limits<fishnet::math::DEFAULT_FLOATING_POINT>::max();
    Vec2D<T> nearest;
    if(points.size()==1)
        return *points.begin();
    //https://www.codeproject.com/Articles/882739/Simple-Approach-to-Voronoi-Diagrams --> stop computation when x difference is greater than current minimum
    for( auto it = (points.lower_bound(query)==points.end()?--points.lower_bound(query):points.lower_bound(query)); std::make_reverse_iterator(it) != points.rend() && fabs(it->x - query.x) < minDistance ; --it){
        const Vec2D<T> & neighbour = *it;
        if(query.distance(neighbour) < minDistance and neighbour != query and predicate(neighbour)){
            minDistance = query.distance(neighbour);
            nearest= neighbour;
        }
    }   
    for( auto it = points.upper_bound(query); it != points.end() && fabs(it->x - query.x) < minDistance; ++it){
        const Vec2D<T> & neighbour = *it;
        if(query.distance(neighbour) < minDistance and neighbour != query and predicate(neighbour)){
            minDistance = query.distance(neighbour);
            nearest = neighbour;
        }
    } 
    return nearest;
}

}

template<math::Number T>
constexpr Vec2D<T> nearestNeighbour(const Vec2D<T> & query, std::set<Vec2D<T>,LexicographicOrder> const & points) noexcept {
    return __impl::nearestNeighbourBinarySearch<T>(query,points,[](const auto & e){return true;});
}


template<math::Number T>
constexpr std::vector<Vec2D<T>> kNearestNeighbours(const Vec2D<T> & query,  std::set<Vec2D<T>,LexicographicOrder>  const & points, const size_t k) noexcept {
        const size_t _k = std::min(k,util::size(points) - 1 );
        std::vector<Vec2D<T>> neighbours;
        neighbours.resize(_k);
        for(size_t i = 0; i < _k; i++){
            auto notYetNeigbourPredicate = [neighbours](const Vec2D<T> & p){return std::ranges::find(neighbours,p) == neighbours.end();};
            neighbours[i] = __impl::nearestNeighbourBinarySearch(query,points,notYetNeigbourPredicate);
        }
        return neighbours;
}

template<typename T>
constexpr std::vector<std::pair<Vec2D<T>,std::vector<Vec2D<T>>>> AllKNearestNeighbours(util::forward_range_of<Vec2D<T>> auto const & points, const size_t k) noexcept {
    std::set<Vec2D<T>,LexicographicOrder> orderedSetOfPoints;
    std::vector<std::pair<Vec2D<T>,std::vector<Vec2D<T>>>> result;
    for(const auto & p: points){
        orderedSetOfPoints.emplace(p);
    }
    for( auto & point : orderedSetOfPoints){
        result.emplace_back(std::make_pair(point,kNearestNeighbours(point,orderedSetOfPoints,k)));
    }
    return result;
}
}
