#ifndef TEST_XYNode_H
#define TEST_XYNode_H
#include <functional>
#include <math.h>
#include <fishnet/Printable.hpp>
class XYNode
{
private:
    double x;
    double y;
public:
    XYNode(double x, double y):x(x),y(y){};

    bool operator==(const XYNode & other) const {
        return this->x == other.x and this->y == other.y;
    }

    double getX() const {
        return this->x;
    }

    double getY()const {
        return this->y;
    }

    double distanceTo(const XYNode & other) const  {
        double deltaX = this->getX() - other.getX();
        double deltaY =this->getY() - other.getY();
        return sqrt(deltaX*deltaX + deltaY*deltaY);
    }

    std::string toString() const {
        return "("+std::to_string(this->x)+","+std::to_string(this->y)+")";
    }
};

static_assert(fishnet::util::Printable<XYNode>, "XYNode should be printable");

namespace std{
    template<>
    struct hash<XYNode>{
        size_t operator()(const XYNode & n) const {
            size_t x_hash = hash<double>{}(n.getX());
            size_t y_hash = hash<double>{}(n.getY());
            return ((x_hash + y_hash+1)* (x_hash+y_hash)) / 2 + y_hash;
        }
    };
}
#endif