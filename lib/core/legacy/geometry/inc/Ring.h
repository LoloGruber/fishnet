//
// Created by grube on 13.01.2022.
//

#ifndef WSF_NETWORK_RING_H
#define WSF_NETWORK_RING_H
#include "Line.h"
/**
 * Util class to define a Ring (list of points, forming non-intersecting lines)
 */
class Ring {
private:
    std::vector<Line> lines;
    std::vector<Vec2D> points;
public:
    Ring() = default;

    Ring(const std::vector<Vec2D> & points,const std::vector<Line> & lines) : points(points), lines(lines){};

    bool addPoint(Vec2D point);

    bool intersects(Line &newLine);

    bool onRing(const Vec2D &point);

    bool insideRing(const Vec2D &point);

    bool contains(const Vec2D &point);

    OGRLinearRing *toOGRLinearRing();

    const std::vector<Vec2D> &getPoints();
};


#endif //WSF_NETWORK_RING_H
