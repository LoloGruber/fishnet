#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/Rectangle.hpp>
#include <fishnet/NumericConcepts.hpp>
namespace fishnet::geometry {
template<IPolygon P>
class BoundingBoxPolygon {
private:
    const P polygon; 
    Rectangle<fishnet::math::DEFAULT_NUMERIC> boundingBox;
public:
    BoundingBoxPolygon(const P & polygon):polygon(polygon),boundingBox(polygon.aaBB().getPoints()){}

    BoundingBoxPolygon(const P & polygon, const Rectangle<fishnet::math::DEFAULT_NUMERIC> & boundingBox):polygon(polygon),boundingBox(boundingBox){}

    const Rectangle<fishnet::math::DEFAULT_NUMERIC> & getBoundingBox() const noexcept {
        return this->boundingBox;
    }

    const P & getPolygon() const noexcept {
        return polygon;
    }
};

template<IPolygon P>
struct VerticalAABBOrdering{
    bool operator()(const BoundingBoxPolygon<P> & lhs, const BoundingBoxPolygon<P> & rhs) const noexcept {
        return lhs.getBoundingBox().top() > rhs.getBoundingBox().top();
    }
};

template<IPolygon P>
struct HorizontalAABBOrdering {
    bool operator()(const BoundingBoxPolygon<P> & lhs, const BoundingBoxPolygon<P> & rhs) const noexcept {
        return lhs.getBoundingBox().left() < rhs.getBoundingBox().left();
    }
};
}