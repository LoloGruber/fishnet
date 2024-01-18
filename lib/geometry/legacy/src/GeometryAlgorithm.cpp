//
// Created by grube on 13.01.2022.
//
#include <unordered_set>
#include <iostream>
#include <set>
#include "GeometryAlgorithm.h"
#include <GeoUtil.h>
#include "Ring.h"

double const GeometryAlgorithm::PI = 3.14159265358979323846;

// https://towardsdatascience.com/the-concave-hull-c649795c0f0f
std::unique_ptr<Polygon>
GeometryAlgorithm::concaveHullApproxImpl(std::unordered_set<Vec2D, Vec2D::Hash> &dataset, int neighbors) {
    std::unordered_set<Vec2D, Vec2D::Hash> backup;
    for (auto &p: dataset) {
        backup.insert(Vec2D(p.x,p.y));
    }
    neighbors = std::max(neighbors, 3);
    neighbors = std::min(neighbors,(int) dataset.size() - 1);
    auto firstPoint = lowestPoint(dataset);
    auto ring = Ring();
    ring.addPoint(firstPoint);
    dataset.erase(firstPoint);
    auto current = firstPoint;
    double previousAngle = PI*3/2; // 270 degrees => using clockwise bearings
    int step = 2;
    while ((current != firstPoint or step == 2) and not dataset.empty()) {
        if (step == 5) {
            dataset.insert(firstPoint);
        }
        auto kNeareset = kNearestNeighbors(dataset, current, previousAngle, neighbors);
        bool intersects = true;
        int i = 0;
        for (auto &n: kNeareset) {
            auto newLine = Line(current, n);
            intersects = ring.intersects(newLine);
            if (not intersects) {
                break;
            } //-7.866906155,33.508237780 -7.866906258,33.508238067
            i++;
        }
        if (intersects) {
            return concaveHullApproxImpl(backup, neighbors + 2);
        }
        auto next = kNeareset[i];
        previousAngle = next.bearing(current,previousAngle);
        current = next;
        ring.addPoint(current);
        dataset.erase(current);
        step++;
    }
    bool allInside = true;
    for (auto &p: dataset) {
        if (not ring.contains(p)) {
            allInside = false;
        }
    }
    if (not allInside and not (neighbors == backup.size()-1)) {
        return concaveHullApproxImpl(backup, neighbors + 2);
    } else {
        auto polygon = OGRPolygon();
        polygon.addRing(ring.toOGRLinearRing());
        return std::make_unique<Polygon>(polygon);
    }

}

std::unique_ptr<Polygon> GeometryAlgorithm::concaveHullApprox(std::vector<Vec2D> &points, int neighbors) {
    auto ds = eliminateDuplicates(points);
    if (ds.size() < 3) {
        return nullptr;
    }
    if (ds.size() == 3) {
        return Polygon::create(points);
    }
    return concaveHullApproxImpl(ds, neighbors);
}

std::vector<Vec2D> GeometryAlgorithm::kNearestNeighbors(std::unordered_set<Vec2D,Vec2D::Hash> &ds, Vec2D current,double angle, int neighbors) {
    auto cmp = [&current] (const Vec2D & u, const Vec2D & v) {
        return current.distance(u) < current.distance(v);
    };
    std::vector<Vec2D> candidates;
    candidates.reserve(ds.size());
    for (auto &p: ds) {
        candidates.push_back(p);
    }
    std::sort(candidates.begin(), candidates.end(), cmp);
    candidates.resize(std::min((std::size_t) neighbors, candidates.size()));
    auto anglecmp = [&current, & angle] (const Vec2D & u, const Vec2D & v) {
        return u.bearing(current, angle) > v.bearing(current, angle);
    };
    std::sort(candidates.begin(), candidates.end(), anglecmp);
    return candidates;
}

Vec2D GeometryAlgorithm::lowestPoint(std::unordered_set<Vec2D, Vec2D::Hash> &inputPoints) {
    Vec2D lowest = Vec2D(0,MAXFLOAT);
    for (auto &p: inputPoints) {
        if (p.y < lowest.y or p.y <= lowest.y and p.x < lowest.x) {
            lowest = p;
        }
    }
    return lowest;
}

std::unordered_set<Vec2D,Vec2D::Hash> GeometryAlgorithm::eliminateDuplicates(std::vector<Vec2D> &points){
    std::unordered_set<Vec2D,Vec2D::Hash> dataset;
    for (auto &p: points) {
        dataset.insert(p);
    }
    return dataset;
}







//std::unique_ptr<Polygon> GeometryAlgorithm::concaveHullApprox(std::vector<Vec2D> &inputPoints, double maxDistance) {
//    /*Calculate lower left point*/
//    auto startingPoint = inputPoints[0];
//    for (auto &p: inputPoints) {
//        if (p.x < startingPoint.x and p.y <= startingPoint.y or p.x <= startingPoint.x and p.y < startingPoint.y) {
//            startingPoint = p;
//        }
//    }
//    /* Store Vectors in a Set, eliminate Duplicates */
//    std::unordered_set<Vec2D, Vec2D::Hash> ps;
//    for (auto &p: inputPoints) {
//        ps.insert(p);
//    }
///*    std::vector<Vec2D> points;
//    points.push_back(startingPoint);*/
//
//    double max = maxDistance;
//    /* Walk outer points of polygon*/
//    auto current = startingPoint;
//    auto result = Ring();
//    result.addPoint(startingPoint);
//    double angle = PI; //intial angle = 180deg -> starting from negativ x-axis, rotating counterclockwise-> bigest angles for -x and +y
//    do {
//        auto cmp = [&angle,&current](const Vec2D& u, const Vec2D & v){//biggest angle store as last / max element
//            auto angleU = u.angle(current, angle);
//            auto angleV = v.angle(current, angle);
//            if ((angleU - angleV) < Vec2D::epsilon) {
//                return current.distance(u) < current.distance(v); //sort inverse by distance => biggest angle, shortest distance as next element
//            }
//            return angleU > angleV;
//        };
//        /* check all still available points, if their distance is smaller than max distance -> canidates for the next edge in the concave hull*/
//        Vec2D candidate= *(ps.begin());
//        for (auto &p: ps) {
//            if (cmp(p,candidate)) {
//                candidate = p;
//            }
//        }
//        /* restart searching for neighboring points with higher distance when no candidates exist or the only candidate is the starting point while still other points are unexplored*/
//        if (candidate == startingPoint and ps.size() >1) {
//            max = max + maxDistance/2;
//            continue;
//        }else if (candidate == startingPoint) {
//            result.addPoint(candidate);
//            break;
//        }
//        /* */
//        if (result.addPoint(candidate)) {
//            max = std::max(max - maxDistance / 2, maxDistance);
//            angle = candidate.angle(current, angle);
//            current = candidate;
//            ps.erase(candidate);
//            if (current == startingPoint) {
//                break;
//            }
//        } else {
//            max = max + maxDistance / 2;
//        }
//
//        /**
//         * -7.8012394217557706
//         * 33.509495717714671
//         */
//    } while (true);
//    auto polygon = OGRPolygon();
//    polygon.addRing(result.toOGRLinearRing());
//    return std::make_unique<Polygon>(polygon);
//}
