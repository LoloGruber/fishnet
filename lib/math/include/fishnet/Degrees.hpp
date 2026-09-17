#pragma once
#include "Angle.hpp"
#include "Radians.hpp"
#include <string>

namespace fishnet::math{
/**
 * @brief Degrees implementation of Angle
 * 
 */
class Degrees final: public __impl::AbstractAngle<Degrees>{
protected:
    double normalizedAngle(const double newAngle) const;
    friend __impl::AbstractAngle<Degrees>;
public:
    static Degrees asin(double sine);
    static Degrees acos(double cosine);
    static Degrees atan(double tangent);
    static Degrees atan2(double y, double x);

    explicit Degrees(double deg);
    explicit Degrees(const Radians & rad);
    Radians toRadians()const;
    double sin() const;
    double cos() const;
    double tan() const;
    std::string toString() const noexcept;
};
} // namespace fishnet::math