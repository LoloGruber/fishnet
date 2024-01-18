//
// Created by grube on 13.01.2022.
//

#ifndef WSF_NETWORK_GEOMETRYALGORITHM_H
#define WSF_NETWORK_GEOMETRYALGORITHM_H
#include "Polygon.h"
#include <unordered_set>

/**
 * [DEPRECATED] will be implemented in the future
 */
class GeometryAlgorithm{
public:
    static const double PI;

    static std::unique_ptr<Polygon> concaveHullApprox(std::vector<Vec2D> &points, int neighbors);
private:
    static std::unique_ptr<Polygon> concaveHullApproxImpl(std::unordered_set<Vec2D, Vec2D::Hash>  &datset, int neighbors);

    static Vec2D lowestPoint(std::unordered_set<Vec2D, Vec2D::Hash> &inputPoints);

    static std::unordered_set<Vec2D,Vec2D::Hash> eliminateDuplicates(std::vector<Vec2D> &points);

    static std::vector<Vec2D> kNearestNeighbors(std::unordered_set<Vec2D,Vec2D::Hash> &ds, Vec2D current,double angle, int neighbors);


};
#endif //WSF_NETWORK_GEOMETRYALGORITHM_H
