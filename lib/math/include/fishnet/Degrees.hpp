#pragma once

#include "Angle.hpp"
#include "Radians.hpp"
#include <ostream>

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

    friend std::ostream& operator<<(std::ostream& os, const Degrees& deg) {
        os << deg.angle << "°";
        return os;
    }
};
}

namespace std {
    template<>
    struct hash<fishnet::math::Degrees>{
        size_t operator()(const fishnet::math::Degrees & deg) const {
            return hash<double>{}(deg.getAngleValue());
        }
    };
}