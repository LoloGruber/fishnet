//
// Created by grube on 14.01.2022.
//


#ifndef WSF_NETWORK_CHARACTERISTICSHAPE_H
#define WSF_NETWORK_CHARACTERISTICSHAPE_H


#include <memory>
#include <unordered_set>
#include "Polygon.h"
#include "CombinatorialMap.h"

/**
 * Based on the paper "Efficient generation of simple polygons for characterizing the shape of a set of points in the plane"
 *
 * http://geosensor.net/papers/duckham08.PR.pdf
 */
class CharacteristicShape {
public:
    static std::vector<Vec2D> charactersticConcaveHull(std::vector<Vec2D> &points, double lambda=0.17); //lambda being how concave the polygon will be -> 0.17 emerged to be a good estimate
private:
//    static CombinatorialMap delaunayTriangulation(std::unordered_set<Vec2D,Vec2D::Hash> &points);
};


#endif //WSF_NETWORK_CHARACTERISTICSHAPE_H
