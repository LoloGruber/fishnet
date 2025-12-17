#include <fishnet/Radians.hpp>
#include <fishnet/Normalize.hpp>

namespace fishnet::math{

Radians::Radians(double rad){
    this->setAngle(rad);
}

Radians::Radians(const Degrees & deg){
    this->setAngle(deg.toRadians().getAngleValue());
}

Degrees Radians::toDegrees()const{
    return Degrees(this->getAngleValue() * RAD_TO_DEG);
}

double Radians::normalizedAngle(const double newAngle)const {
    return normalize(newAngle,TWO_PI);
}

double Radians::sin() const {
    return std::sin(angle);
}

double Radians::cos() const {
    return std::cos(angle);
}

double Radians::tan() const {
    return std::tan(angle);
}

Radians Radians::asin(double sine){
    return Radians(std::asin(sine));
}

Radians Radians::acos(double cosine){
    return Radians(std::acos(cosine));
}

Radians Radians::atan(double tangent){
    return Radians(std::atan(tangent));
}

Radians Radians::atan2(double y, double x){
    return Radians(std::atan2(y,x));
}
}


