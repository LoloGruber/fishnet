//
// Created by grube on 06.01.2022.
//

#include "Line.h"

double Line::length() {
    return (v1 - v2).length();
}

/*
OGRPolygon *Line::createGeometry(double width) {
    if (width <= 0) {
        throw std::invalid_argument("Width of Line has to be greater than 0!");
    }
    auto direction = v2 - v1;
    auto center = v1 + (direction / 2);
    auto orthogonal = direction.orthogonal().normalize();

    auto p1 = (v1 + (orthogonal * width / 2));
    auto p2 = (v1 - (orthogonal * width / 2));
    auto p3 = (v2 + (orthogonal * width / 2));
    auto p4 = (v2 - (orthogonal * width / 2));

    auto cmpClockwise = [&center](Vec2D & u, Vec2D & w) {
        return u.angle(center) > w.angle(center);
    };
    std::vector<Vec2D> vectors = {p1, p2, p3, p4};
    std::sort(vectors.begin(), vectors.end(), cmpClockwise);
    auto * ring = new OGRLinearRing();
    for (auto &vector: vectors) {
        ring->addPoint(vector.toOGRPoint());
    }
    ring->addPoint(vectors[0].toOGRPoint());
    auto polygon = new OGRPolygon();
    polygon->addRing(ring);
    return polygon;
}
*/

Line::Line(const Vec2D &v1, const Vec2D &v2) {
    this->v1 = v1;
    this->v2 = v2;
}

Vec2D Line::intersection(Line &other) {
    if (isParallel(other)) {
        throw std::invalid_argument("Lines are parallel! No intersection");
    }
    auto dThis = direction();
    // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
    double denominator = (v1.x - v2.x) * (other.v1.y - other.v2.y) - (v1.y - v2.y) *(other.v1.x - other.v2.x);
    double lambda =
            ((v1.x - other.v1.x) * (other.v1.y - other.v2.y) - (v1.y - other.v1.y) * (other.v1.x - other.v2.x)) /
            denominator;
//    double my = ((v1.x - other.v1.x) * (v1.y - v2.y) - (v1.y - other.v1.y) * (v1.x - v2.x)) / denominator;
    return v1 + (dThis * lambda);
}

bool Line::isParallel(Line &other) {
    auto dir1 = direction();
    auto dir2 = other.direction();
    return dir1.isParallel(dir2);
}

bool Line::onLine(Vec2D &point) {
    auto dThis = direction();
    if (dThis.x == 0 and dThis.y == 0) {
        return v1 == point;
    } else if (dThis.x == 0) {
        double lambdaY = (point.y - v1.y) / dThis.y;
        return lambdaY >= 0.0-epsilon/2 and lambdaY <= 1.0 + epsilon/2 and v1.x == point.x; //lambda has to be between 0 and 1 and the x coord has to be equal
    } else if (dThis.y == 0) {
        double lambdaX = (point.x - v1.x) / dThis.x;
        return lambdaX >= 0.0-epsilon/2 and lambdaX <= 1.0+epsilon/2 and v1.y == point.y;  //lambda has to be between 0 and 1 and the y coord has to be equal
    } else {
        double lambdaX = (point.x - v1.x) / dThis.x;
        double lambdaY = (point.y - v1.y) / dThis.y;
        return abs(lambdaY - lambdaX) < epsilon and lambdaX <= 1.0 and lambdaX >= 0.0; //both lambdas have to come to same result and be between 0 and 1 for the point being on the line
    }
}

bool Line::valid() {
    return length() > 0;
}

Vec2D Line::directionNormalized() {
    return direction().normalize();
}

Vec2D Line::direction() {
    return v2 - v1;
}

Vec2D Line::getFixpoint() {
    return v1;
}

Vec2D Line::getReferencePoint() {
    return v2;
}

bool Line::intersectionOnLine(Line &other) {
    if (isParallel(other)) {
        return onLine(other.v1) or onLine(other.v2);
    }else{
        auto intersection = this->intersection(other);
        return this->onLine(intersection);
    }
}

bool Line::touches(Line &other) {
    return this->v1 == other.v1 or this->v1 == other.v2 or this->v2 == other.v1 or this->v2 == other.v1;
}

bool Line::equalNoOrientation(Line &other) {
    return this->v1 == other.v1 and this->v2 == other.v2 or this->v1 == other.v2 and this->v2 == other.v1;
}



