//
// Created by grube on 14.01.2022.
//

#ifndef WSF_NETWORK_POLYGONMERGESTRATEGY_H
#define WSF_NETWORK_POLYGONMERGESTRATEGY_H
#include <memory>
#include "utils/Geometry/Polygon.h"
/**
 * Functional Interface for defining the merge of two polygons into a single polygon
 */
class PolygonMergeStrategy{
public:
    virtual std::unique_ptr<Polygon> merge(const Polygon &u, const Polygon &) = 0;
};
#endif //WSF_NETWORK_POLYGONMERGESTRATEGY_H
