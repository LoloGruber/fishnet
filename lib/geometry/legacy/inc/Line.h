//
// Created by grube on 06.01.2022.
//

#ifndef BACHELORARBEIT_LINE_H
#define BACHELORARBEIT_LINE_H
//#include "ogr_api.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"
#include "cpl_conv.h"
#include "Vec2D.hpp"
#include <ogr_core.h>
/**
 * This class represents lines constructed from two Vec2D points. v1 is the fixpoint and v2 is the reference point
 * The direction of the line is defined by v2-v1 (==direction vector)
 */
class Line {
private:
    Vec2D v1{};
    Vec2D  v2{};
    constexpr static const double epsilon = 0.00000001;
public:

    Line(const Vec2D & fixpoint, const Vec2D & referencePoint);

    Line()=default;

    Vec2D getFixpoint();

    Vec2D getReferencePoint();

    /**
     * Calculates the intersection between <this> and the given Line <other>
     * @throws InvalidArugmentExecption when the lines are parallel
     * @param other
     * @return Position of Intersection
     */
    Vec2D intersection(Line &other);

    /**
     *
     * @param other
     * @return whether this and other are parallel
     */
    bool isParallel(Line &other);

    /**
     *
     * @param other
     * @return whether the intersection of <this> and <other> is located on <this> line (between v1 and v2)
     */
    bool intersectionOnLine(Line &other);

    /**
     *
     * @return whether length is greater than 0
     */
    bool valid();

    /**
     *
     * @param point
     * @return whether the given point is located on the line
     */
    bool onLine(Vec2D &point);


    /**
     *
     * @param other
     * @return the lines touch each other: fix or reference point are the same
     */
    bool touches(Line &other);

    /**
     * Check for Equality, disregaring orientation:
     * Line(a,b) == Line(b,a)
     * @param other
     * @return
     */
    bool equalNoOrientation(Line & other);

    /**
     *
     * @return direction vector (v2-v1)
     */
    Vec2D direction();

    /**
     * Normalized direction vector
     * @return
     */
    Vec2D directionNormalized();


    /**
     * Length of Line -> length of direction vector
     * @return
     */
    double length();


};


#endif //BACHELORARBEIT_LINE_H
