#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/Rectangle.hpp>
#include <fishnet/NumericConcepts.hpp>
namespace fishnet::geometry {

/**
 * @brief Wrapper for Polygon objects, which stores a (custom) bounding box 
 * 
 * @tparam P polygon type
 */
template<IPolygon P>
class BoundingBoxPolygon {
private:
    const P polygon; 
    Rectangle<fishnet::math::DEFAULT_NUMERIC> boundingBox;
public:
    /**
     * @brief Construct a new Bounding Box Polygon object, with default axis-aligned bounding box
     * 
     * @param polygon 
     */
    BoundingBoxPolygon(const P & polygon):polygon(polygon),boundingBox(polygon.aaBB().getPoints()){}

    /**
     * @brief Construct a new Bounding Box Polygon object, with custom bounding box rectangle
     * 
     * @param polygon 
     * @param boundingBox 
     */
    BoundingBoxPolygon(const P & polygon, const Rectangle<fishnet::math::DEFAULT_NUMERIC> & boundingBox):polygon(polygon),boundingBox(boundingBox){}

    const Rectangle<fishnet::math::DEFAULT_NUMERIC> & getBoundingBox() const noexcept {
        return this->boundingBox;
    }

    const P & getPolygon() const noexcept {
        return polygon;
    }
};

/**
 * @brief Helper function to get the common bounding box for a range of polygons
 * 
 * @tparam R type of polygon range
 * @param polygons polygons to compute the bounding box around
 * @return Rectangle<typename std::ranges::range_value_t<R>::numeric_type> BoundingBox using the numeric type like the input
 */
template<PolygonRange R>
static Rectangle<typename std::ranges::range_value_t<R>::numeric_type> minimalBoundingBox(const R & polygons) {
    using number = typename std::ranges::range_value_t<R>::numeric_type;
    number left = std::numeric_limits<number>::max();
    number right = std::numeric_limits<number>::min();
    number top = std::numeric_limits<number>::min();
    number bottom = std::numeric_limits<number>::max();
    if(util::isEmpty(polygons)){
        throw std::runtime_error("Range of polygons is empty, cannot compute bounding box");
    }
    for(const auto & polygon: polygons){
        auto aaBB = Rectangle<number>(polygon);
        if(aaBB.left() < left)
            left = aaBB.left();
        if(aaBB.right() > right)
            right = aaBB.right();
        if(aaBB.top() > top)
            top = aaBB.top();
        if(aaBB.bottom() < bottom)
            bottom = aaBB.bottom();
    }
    return Rectangle<number>({{left,top},{right,top},{right,bottom},{left,bottom}});
}

/**
 * @brief Comparator for sorting BoundingBoxPolygons according to top-most position of the bounding box
 * 
 * @tparam P 
 */
template<IPolygon P>
struct VerticalAABBOrdering{
    bool operator()(const BoundingBoxPolygon<P> & lhs, const BoundingBoxPolygon<P> & rhs) const noexcept {
        return lhs.getBoundingBox().top() > rhs.getBoundingBox().top();
    }
};

/**
 * @brief Comparator for sorting BoundingBoxPolygons according to left-most position of the bounding box
 * 
 * @tparam P 
 */
template<IPolygon P>
struct HorizontalAABBOrdering {
    bool operator()(const BoundingBoxPolygon<P> & lhs, const BoundingBoxPolygon<P> & rhs) const noexcept {
        if(lhs.getBoundingBox().left() == rhs.getBoundingBox().left()){
            return lhs.getBoundingBox().right() < rhs.getBoundingBox().right();
        }
        return lhs.getBoundingBox().left() < rhs.getBoundingBox().left();
    }
};
}