//
// Created by grube on 29.09.2021.
//

#ifndef BACHELORARBEIT_POLYGON_H
#define BACHELORARBEIT_POLYGON_H
#include "gdal_priv.h"
#include "gdal.h"
#include <memory>
#include "ogr_api.h"
#include "ogr_spatialref.h"
#include "ogrsf_frmts.h"
#include "cpl_conv.h"
#include "Vec2D.hpp"
#include "Line.h"
#include <ogr_core.h>

/**
 * This class is wrapper for the GDAL OGRPolygon class, decoupling GDAL boilerplate code from the Settlement class
 */
class Polygon {
private:
    std::unique_ptr<OGRGeometry> polygon; // reference to OGRPolygon object
    std::vector<Vec2D> points;

    /**
     * Helper method to create OGRGeometry from <points>
     */
    void createGeometry();

public:
    explicit Polygon(OGRPolygon & ogrPolygon) {
        polygon = std::make_unique<OGRPolygon>(ogrPolygon);
        for (auto & p: this->polygon->toPolygon()->getExteriorRing()) {
            points.emplace_back(p.getX(), p.getY());
        }
    };

    const std::unique_ptr<OGRGeometry> & getGeometry() const;

    /**
     * @return vector of points, which form the exterior ring of the polygon
     */
    [[nodiscard]]const  std::vector<Vec2D>  & getPoints() const;


    /**
     * @return lines contructed from the points of the outer ring of the polygon
     */
    [[nodiscard]] std::vector<Line> getLines() const;


    /**
     * Wrapper for OGRPolygon methods:
     */
    bool touches(Polygon const &other);

    bool intersects(Polygon const &other);

    bool overlaps(Polygon const &other);

    bool contains(Polygon const &other) const;

    std::unique_ptr<Polygon> unionPolygons(Polygon const & other);

    /**
     * Polygons are equal if their OGRPolygon is equal
     * @param other
     * @return
     */
    bool operator==(Polygon const &other);


    /**
     *
     * @return approx. area  of Polygon in meters^2
     */
    double area() const;

    /**
     * @param other
     * @return distance in meters to the other polygon
     */
    double distance(Polygon const&other) const;

    /**
     * Calls OGRPolygon->Centroid()
     * @return Vec2D with the coordinates of the centroid location
     */
    [[nodiscard]] Vec2D centroid() const;







    /**
     * Factory method to create a Polygon obj
     * @param feature from which the OGRPolygon will be extracted, if MultiPolygon -> compute concaveHull
     * @return
     */
    static std::unique_ptr<Polygon> create(OGRFeature &feature);


    /**
     * Factory method to create a Polygon obj from a list of points in clockwise order
     * @param pointVector
     * @return
     */
    static std::unique_ptr<Polygon> create(std::vector<Vec2D> &pointVector);

    /**
     *
     * @param feature polygon or multipolygon
     * @return list of polygons contained in the feature (1 or multiple for multipolygons)
     */
    static std::vector<std::unique_ptr<Polygon>> createFromMultiPolygon(OGRFeature & feature);


};


#endif //BACHELORARBEIT_POLYGON_H
