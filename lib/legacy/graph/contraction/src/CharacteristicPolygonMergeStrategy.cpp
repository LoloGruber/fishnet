//
// Created by grube on 14.01.2022.
//

#include <iostream>
#include "CharacteristicPolygonMergeStrategy.h"
#include "utils/Geometry/CharateristicShape/CharacteristicShape.h"

std::unique_ptr<Polygon> CharacteristicPolygonMergeStrategy::merge(const Polygon &u, const Polygon &v) {
    std::vector<Vec2D> combinedPoints;
    for (auto &p: u.getPoints()) {
        combinedPoints.push_back(p);
    }
    for (auto &p: v.getPoints()) {
        combinedPoints.push_back(p);
    }
    try {
        /* Delegated compuation to Helper Class to create characteristic shape of a set of points*/
        auto result = CharacteristicShape::charactersticConcaveHull(combinedPoints);
        auto polygon = Polygon::create(result);
        /* Execute on failure: return bigger polygon*/
        if (polygon == nullptr) {
            if (u.getPoints().size() > v.getPoints().size()) {
                return std::make_unique<Polygon>(*u.getGeometry()->toPolygon());
            } else {
                return std::make_unique<Polygon>(*v.getGeometry()->toPolygon());
            }
        } else {
            return polygon;
        }
    } catch (std::out_of_range &ofRange) {
        /* Execute on failure: return bigger polygon*/
        if (u.getPoints().size() > v.getPoints().size()) {
            return std::make_unique<Polygon>(*u.getGeometry()->toPolygon());
        } else {
            return std::make_unique<Polygon>(*v.getGeometry()->toPolygon());
        }
    }
}
