#include <cmath>
#include <fishnet/Degrees.hpp>
#include <fishnet/Normalize.hpp>
namespace fishnet::math{

Degrees::Degrees(double deg){
    this->setAngle(deg);
}

Degrees::Degrees(const Radians & rad){
    this->setAngle(rad.toDegrees().getAngleValue());
}

Radians Degrees::toRadians() const {
    return Radians(this-> getAngleValue() * DEG_TO_RAD);
}

double Degrees::normalizedAngle(const double newAngle) const {
    return normalize(newAngle,360.0);
}

double Degrees::sin() const{
    return std::sin(angle*DEG_TO_RAD);
}

double Degrees::cos() const {
    return std::cos(angle*DEG_TO_RAD);
}

double Degrees::tan() const {
    return std::tan(angle*DEG_TO_RAD);
}

Degrees Degrees::asin(double sine){
    return Degrees(std::asin(sine)*RAD_TO_DEG);
}

Degrees Degrees::acos(double cosine){
    return Degrees(std::acos(cosine) * RAD_TO_DEG);
}

Degrees Degrees::atan(double tangent){
    return Degrees(std::atan(tangent)*RAD_TO_DEG);
}

Degrees Degrees::atan2(double y, double x){
    return Degrees(std::atan2(y,x)*RAD_TO_DEG);
}
}