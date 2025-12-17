//
// Created by grube on 06.01.2022.
//

#include <valarray>
#include "Vec2D.h"
#include "GeometryAlgorithm.h"


Vec2D Vec2D::operator*(double scalar) const {
    return {x*scalar, y*scalar};
}

Vec2D Vec2D::operator+(const Vec2D &other) const {
    return {x + other.x, y + other.y};
}

Vec2D Vec2D::normalize() const {
    return *this * (1/length());
}

double Vec2D::length() const {
    return sqrt(dot(*this));
}

double Vec2D::dot(const Vec2D &other) const {
    return x * other.x + y * other.y; // dot product of two vectors
}

Vec2D Vec2D::operator/(double scalar) const {
    if (scalar == 0) {
        return *this;
    }else{
        return {x / scalar, y / scalar};
    }
}

Vec2D Vec2D::operator-(const Vec2D &other) const {
    return {x - other.x, y - other.y};
}

bool Vec2D::operator==(const Vec2D &other) const {
    return abs(x - other.x) <epsilon and abs(y - other.y) < epsilon;
}

Vec2D Vec2D::orthogonal() const {
    return {y, - x}; //clockwise orthogonal vector
}

OGRPoint *Vec2D::toOGRPoint() const {
    return new OGRPoint(x, y);
}
/**
 * Computes the angle (counterclockwise) for the direction vector with reference to the reference vector
 * @param reference for the angle
 * @return
 */
double Vec2D::angle(const Vec2D &reference) const {
    Vec2D direction = *this - reference;
    if (direction.length() == 0) {
        throw std::invalid_argument("Not supported for coinciding points (length of direction vector is zero)");
    }
    return atan2(direction.y, direction.x);
}

double Vec2D::angle(const Vec2D &reference, double angleRotate) const {
    auto shiftedAngle = (angle(reference) + angleRotate);
    if ((shiftedAngle - GeometryAlgorithm::PI * 2) > epsilon/2) {
        return shiftedAngle - GeometryAlgorithm::PI * 2;
    }
    return shiftedAngle;
}

double Vec2D::bearing(const Vec2D &reference, double angleRotate) const {
    double delta = this->x - reference.x;
    double yVal = sin(delta) * cos(this->y);
    double xVal = cos(reference.y) * sin(this->y) - sin(reference.y) * cos(delta) * cos(this->y);
    double bearing = (atan2(yVal, xVal) + GeometryAlgorithm::PI * 2); //clockwise angle
    if (bearing < GeometryAlgorithm::PI * 2) {
        bearing += GeometryAlgorithm::PI * 2;
    }
    bearing = bearing - angleRotate; //adjust for current bearing with angleRotate
    if (bearing < 0.0) {
        bearing += GeometryAlgorithm::PI * 2;
    }
    return bearing;
}




std::string Vec2D::toString() const {
    return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
}

bool Vec2D::isParallel(const Vec2D &other) const {
     return det(other) == 0; // parallel if det(this,other) = 0
}

double Vec2D::det(const Vec2D &other) const {
    return (x * other.y - y * other.x); // determinate of a 2x2 matrix
}

bool Vec2D::operator!=(const Vec2D &other) const {
    return not (*this == other);
}

double Vec2D::distance(const Vec2D &other) const {
    return (*this - other).length();
}

double Vec2D::cross(const Vec2D &other) const {
    return det(other); //cross product == determinate in two dimensional space
}

Vec2D::Vec2D(double x, double y) : x(x),y(y)  {
    if (abs(x) < epsilon) {
        this->x = 0.0;
    }
    if (abs(y) < epsilon) {
        this->y = 0.0;
    }
}





/*Vec2D p = Vec2D(1, 0);
auto u = Vec2D(0, -1);
auto v = Vec2D(-1, 0);
auto w = Vec2D(0, 1);
auto refer = Vec2D(0, 0);
std::cout << p.angle(refer) << std::endl;
std::cout << u.angle(refer) << std::endl;
std::cout << v.angle(refer) << std::endl;
std::cout << w.angle(refer) << std::endl;
auto cmp = [&refer](Vec2D & u, Vec2D & w) {
    return u.angle(refer) > w.angle(refer);
};
std::vector<Vec2D> vectors;
vectors.push_back(p);
vectors.push_back(u);
vectors.push_back(w);
vectors.push_back(v);
for (auto vec: vectors) {
    std::cout << vec.toString() << std::endl;
}
std::cout << std::endl;
std::sort(vectors.begin(), vectors.end(), cmp);
for (auto vec: vectors) {
    std::cout << vec.toString() << std::endl;
}*/


