//
// Created by grube on 13.01.2022.
//

#include "Ring.h"

bool Ring::addPoint(Vec2D point) {

    if (not lines.empty()) {
        //Create new line when possible
        Line newLine = Line(points[points.size() - 1], point);

        if (not intersects(newLine)) {
            lines.push_back(newLine);
            points.push_back(point);
            return true;
        }
        return false;
    }
    if (points.size() == 1) {
        lines.emplace_back(points[0], point);
    }
    points.push_back(point);
    return true;
}



bool Ring::intersects(Line &newLine) {
    for (auto &l: lines) {
        if (l.intersectionOnLine(newLine) and not l.touches(newLine)) {
            return true;
        }
    }
    return false;
}

OGRLinearRing *Ring::toOGRLinearRing() {
    auto *linearRing = new OGRLinearRing();
    for (auto &p: points) {
        linearRing->addPoint(p.toOGRPoint());
    }
    if (not linearRing->isClockwise()) {
        linearRing->reverseWindingOrder();
    }
    return linearRing;
}

const std::vector<Vec2D> &Ring::getPoints() {
    return this->points;
}

bool Ring::onRing(const Vec2D &point) {
    for (auto &p: points) {
        if (p == point) {
            return true;
        }
    }
    return false;
}

bool Ring::insideRing(const Vec2D &point) { //https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
    Line toInfX = Line(point, Vec2D(point.x + 181.0, point.y));
    int intersectionCounter = 0;
    for (auto &line: lines) {
        if (line.intersectionOnLine(toInfX)) {
            intersectionCounter++;
            if (intersectionCounter == 1) {
                auto intersection = line.intersection(toInfX);
                for (auto &p: points) {
                    if (p == intersection) {
                        return false;
                    }
                }
            }
        }
        if (intersectionCounter >= 2) {
            return false;
        }
    }
    return true;

}

bool Ring::contains(const Vec2D &point) {
    return insideRing(point) or onRing(point);
}
