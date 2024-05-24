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