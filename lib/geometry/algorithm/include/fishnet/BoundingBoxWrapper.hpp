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
template<typename T, class ShapeProjection=std::identity> requires geometry::Shape<std::invoke_result_t<ShapeProjection,T>>
class BoundingBoxWrapper {
private:
    const T element; 
    Rectangle<fishnet::math::DEFAULT_NUMERIC> boundingBox;
    static inline ShapeProjection shapeProjection{};
public:
    using element_type = T;
    using geometry_type = std::invoke_result_t<ShapeProjection,T>;
    /**
     * @brief Construct a new Bounding Box Polygon object, with default axis-aligned bounding box
     * 
     * @param element 
     */
    BoundingBoxWrapper(const T & element):element(element),boundingBox(shapeProjection(element)){}

    /**
     * @brief Construct a new Bounding Box Polygon object, with custom bounding box rectangle
     * 
     * @param polygon 
     * @param boundingBox 
     */
    BoundingBoxWrapper(const T & element, const Rectangle<fishnet::math::DEFAULT_NUMERIC> & boundingBox):element(element),boundingBox(boundingBox){}

    const Rectangle<fishnet::math::DEFAULT_NUMERIC> & getBoundingBox() const noexcept {
        return this->boundingBox;
    }

    auto getPolygon() const noexcept {
        return shapeProjection(this->element);
    }

    const T & getElement() const noexcept {
        return this->element;
    }
};

template<typename T>
concept IBoundingBoxWrapper = requires(T t){
    {t.getBoundingBox()} -> std::convertible_to<Rectangle<fishnet::math::DEFAULT_NUMERIC>>;
    {t.getPolygon()} -> geometry::Shape;
    typename T::element_type;
    typename T::geometry_type;
};

/**
 * @brief Comparator for sorting BoundingBoxWrappers according to top-most position of the bounding box
 * 
 * @tparam BBWrapper 
 */
template<IBoundingBoxWrapper BBWrapper>
struct VerticalAABBOrdering{
    bool operator()(const BBWrapper & lhs, const BBWrapper & rhs) const noexcept {
        return lhs.getBoundingBox().top() > rhs.getBoundingBox().top();
    }
};

/**
 * @brief Comparator for sorting BoundingBoxWrappers according to left-most position of the bounding box
 * 
 * @tparam BBWrapper 
 */
template<IBoundingBoxWrapper BBWrapper>
struct HorizontalAABBOrdering {
    bool operator()(const BBWrapper & lhs, const BBWrapper & rhs) const noexcept {
        if(lhs.getBoundingBox().left() == rhs.getBoundingBox().left()){
            return lhs.getBoundingBox().right() < rhs.getBoundingBox().right();
        }
        return lhs.getBoundingBox().left() < rhs.getBoundingBox().left();
    }
};
}