//
// Created by grube on 29.09.2021.
//

#include "Polygon.h"
#include <gdal_priv.h>
#include "gdal.h"
#include "cpl_conv.h" // for CPLMalloc()
#include <gdal_alg.h>
#include <ogr_core.h>
#include <iostream>
#include "ogrsf_frmts.h"
#include "GeoUtil.h"
#include "GeometryAlgorithm.h"
#include "CharacteristicShape.h"

const std::vector<Vec2D> & Polygon::getPoints() const{
    return this->points;
}

const std::unique_ptr<OGRGeometry> & Polygon::getGeometry() const{
/*    if (this->polygon == nullptr) {
        createGeometry();
    }*/
    return this->polygon;
}

void Polygon::createGeometry() {
    auto *ring = new OGRLinearRing();
    for (auto& p: points) {
        ring->addPoint(p.toOGRPoint());
    }
    if (points[0] != points[points.size() - 1]) {
        ring->addPoint(points[0].toOGRPoint());
    }
    auto p = new OGRPolygon();
    this->polygon = std::make_unique<OGRPolygon>();
    this->polygon->toPolygon()->addRing(ring);
}

double Polygon::distance(Polygon const&other) const {
    Vec2D shortestThis = Vec2D();
    Vec2D shortestOther = Vec2D();
    double minDistance = MAXFLOAT;
    // Find points which have the minimum distance
    for (auto& p: this->getPoints()) {
        for (auto& q: other.getPoints()) {
            if (p.distance(q) < minDistance) {
                minDistance = p.distance(q);
                shortestThis = p;
                shortestOther = q;
            }
        }
    }
    if (shortestOther == shortestThis) {
        return 0;
    }
    return GeoUtil::distance(shortestThis.x,shortestThis.y, shortestOther.x,shortestOther.y); //distance in meters
}

std::unique_ptr<Polygon> Polygon::create(OGRFeature &feature) {
    OGRGeometry *geometry = feature.GetGeometryRef();
    if (geometry && wkbFlatten(geometry->getGeometryType() == wkbPolygon)) {
        return std::make_unique<Polygon>(*geometry->toPolygon());
    }
    if (geometry && wkbFlatten(geometry->getGeometryType() == wkbMultiPolygon)) {
        /* Merge Multipolygon using the Characteristic Shape*/
        std::vector<Vec2D> points;
        /* Get all points of the multipolygon*/
        for (auto &polygonEntry: geometry->toMultiPolygon()) {
            for (auto &p: polygonEntry->getExteriorRing()) {
                points.emplace_back(p.getX(), p.getY());
            }
        }
        auto pointsHull = CharacteristicShape::charactersticConcaveHull(points);
        return Polygon::create(pointsHull);
    }

    return nullptr;
}

bool Polygon::contains(const Polygon &other) const{
    return this->polygon->Contains(&*other.polygon);
}

Vec2D Polygon::centroid() const {
    auto p =  OGRPoint();
    auto success = this->polygon->Centroid(&p);
    return {p.getX(), p.getY()};
}

bool Polygon::touches(const Polygon &other) {
    return this->polygon->Touches(&*other.polygon);
}


double Polygon::area() const{
    auto indexMax = points.size() - 1;
    if (points[0] == points[points.size() - 1]) {
        indexMax = points.size() - 2;
    }
    //Calculate the cross product over all points divided by 2
    double area = 0;
    for (int i = 0; i <= indexMax; i++) {
        area += points[i].cross(points[i + 1]); //https://en.wikipedia.org/wiki/Polygon#Area
    }
    area = std::abs(area);

    double distanceInDeg = points[0].distance(points[points.size()/2]);
    double distanceInMeters = GeoUtil::distance(points[0].x,points[0].y, points[points.size()/2].x,points[points.size()/2].y);
    double squaredRatio = pow(distanceInMeters,2) / pow(distanceInDeg,2); // squared ratio for meters in degree -> convert area to approx. m^2
    return (area / 2) * squaredRatio;
}

std::vector<Line> Polygon::getLines() const {
    std::vector<Line> lines;
    auto indexMax = points.size() - 1;
    if (points[0] == points[points.size() - 1]) {
        indexMax = points.size() - 2;
    }
    lines.reserve(indexMax);
    for (int i = 0; i < indexMax; i++) {
        lines.emplace_back(points[i], points[i + 1]);
    }
    lines.emplace_back(points[indexMax], points[0]);
    return lines;
}

bool Polygon::operator==(const Polygon &other) {
    return this->polygon == other.polygon;
}

std::unique_ptr<Polygon> Polygon::create(std::vector<Vec2D> &pointVector) {
    auto *ring = new OGRLinearRing();
    for (auto &vec: pointVector) {
        ring->addPoint(vec.toOGRPoint());
    }
    if (not ring->isClockwise()) {
        ring->reversePoints();
        if (not ring->isClockwise()) {
            return nullptr;
        }
    }
    int i = 0;
    if (not ring->IsSimple() or not ring->IsValid()) {
        if(not ring->IsSimple()){
            i = 1;
        }
        if(not ring->IsValid()){
            i = 2;
        }
        return nullptr;
    }
    auto* polygonGeometry = new OGRPolygon();
    polygonGeometry->toPolygon()->addRing(ring);
    auto polygonValid = polygonGeometry->MakeValid()->toPolygon();
    if (polygonValid and polygonValid->IsValid() and polygonValid->IsSimple()) {
        return std::make_unique<Polygon>(*(polygonValid));
    }
    return nullptr;
}



bool Polygon::overlaps(const Polygon &other) {
    return this->polygon->Overlaps(&*other.polygon) and not this->polygon->Touches(&*other.polygon);
}

std::unique_ptr<Polygon> Polygon::unionPolygons(const Polygon &other) {
    auto unionPolygons = this->polygon->Union(&*other.polygon);
    if (unionPolygons == nullptr) {
        return nullptr;
//        OGRMultiPoint r = OGRMultiPoint();
//        for (auto &p: this->getPoints()) {
//            r.addGeometryDirectly(p.toOGRPoint());
//        }
//        auto rJson = r.exportToJson();
//        std::cout << rJson <<std::flush;
//        OGRMultiPoint s = OGRMultiPoint();
//        for (auto &q: other.getPoints()) {
//            s.addGeometryDirectly(q.toOGRPoint());
//        }
//        auto sJson = s.exportToJson();
//        std::cout << std::endl;
//        std::cout << sJson<< std::flush;
    }
    return std::make_unique<Polygon>(*unionPolygons->toPolygon());
}

std::vector<std::unique_ptr<Polygon>> Polygon::createFromMultiPolygon(OGRFeature &feature) {
    /* Returns one (for wkbPolygon) or multiple polygons (for wkbMultiPolygon) in a list*/
    std::vector<std::unique_ptr<Polygon>> polygons;
    OGRGeometry *geometry = feature.GetGeometryRef();
    if (geometry && wkbFlatten(geometry->getGeometryType() == wkbPolygon)) {
        polygons.push_back(std::make_unique<Polygon>(*geometry->toPolygon()));
    }else if (geometry && wkbFlatten(geometry->getGeometryType() == wkbMultiPolygon)) {
        std::vector<Vec2D> points;
        for (auto &polygonEntry: geometry->toMultiPolygon()) {
            polygons.push_back(std::make_unique<Polygon>(*polygonEntry));
        }
    }
    return polygons;
}

bool Polygon::intersects(const Polygon &other) {
    return this->polygon->Intersects(&*other.polygon);
}



