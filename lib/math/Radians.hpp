#pragma once 

#include "Angle.hpp"
#include "Degrees.hpp"
#include <cmath>
#include <ostream>
#include <functional>

namespace fishnet::math{

class Radians final: public __impl::AbstractAngle<Radians>{
protected:
    double normalizedAngle(const double newAngle) const;
    friend __impl::AbstractAngle<Radians>;
public:
    static Radians asin(double sine);
    static Radians acos(double cosine);
    static Radians atan(double tangent);
    static Radians atan2(double y, double x);
    const static Radians PI;

    explicit Radians(double rad);
    explicit Radians(const Degrees &  deg);
    Degrees toDegrees() const;
    double sin()const;
    double cos()const;
    double tan()const;
    friend std::ostream& operator<<(std::ostream& os, const Radians& radians) {
        os << radians.angle << " [rad]";
        return os;
    }
}; 
const inline Radians Radians::PI = Radians(fishnet::math::PI);
} 
namespace std{
    template<>
    struct hash<fishnet::math::Radians>{
        size_t operator()(const fishnet::math::Radians & rad) const {
            auto val = hash<double>{}(rad.getAngleValue());
            return val;
        }
    };
}



