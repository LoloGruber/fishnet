//
// Created by grube on 08.01.2022.
//

#ifndef BACHELORARBEIT_TWOSHORTESTLINESMERGESTRATEGY_H
#define BACHELORARBEIT_TWOSHORTESTLINESMERGESTRATEGY_H
#include "graph/contraction/PolygonMergeStrategies/PolygonMergeStrategy.h"

/**
 * MergeStrategy based on connecting two polygons with two shortest lines, while elminating susequent interior vertices
 * [DEPRECATED]
 */
class TwoShortestLinesMergeStrategy : public PolygonMergeStrategy{
private:
    /**
     * Helper method to identiy if point is in vector
     * @param points
     * @param point
     * @return
     */
    static bool contains(std::vector<Vec2D> &points, Vec2D &point);

    /**
     * Inserts point without duplicate
     * @param points
     * @param point
     */
    static void insert(std::vector<Vec2D> &points, Vec2D &point);
public:
    std::unique_ptr<Polygon> merge(const Polygon &u, const Polygon &v) override;
};


#endif //BACHELORARBEIT_TWOSHORTESTLINESMERGESTRATEGY_H
