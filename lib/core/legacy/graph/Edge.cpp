//
// Created by grube on 04.01.2022.
//

#include "Edge.h"

const std::shared_ptr<Weight> &Edge::getWeight() {
    return Edge::weight;
}

Edge::Edge(const std::shared_ptr<Settlement> &from, const std::shared_ptr<Settlement> &to, bool directed) {
    this->from = from;
    this->to = to;
    this->directed = directed;
    this->distance = from->distance(*to); //compute distance once,since edges are const!
}

const std::shared_ptr<Settlement>& Edge::getFrom() const {
    return this->from;
}

const std::shared_ptr<Settlement>& Edge::getTo() const {
    return this->to;
}

double Edge::getDistance() const{
    return this->distance;
}

bool Edge::isDirected() const {
    return this->directed;
}

bool Edge::operator==(const Edge &other) const {
    if (this->isDirected() and other.isDirected()) {
        return this->to == other.to and this->from == other.from;
    } else if (not this->isDirected() and not other.isDirected()) {
        return (this->to == other.to and this->from == other.from) or this->to == other.from and this->from == other.to;
    }
    return false; //exactly on of the edges is directed -> not equal
}

bool Edge::contains(const std::shared_ptr<Settlement> &settlement) const{
    return this->to == settlement or this->from == settlement;
}

double Edge::weigth(const std::shared_ptr<Weight> &weightFunction) {
    if (weightFunction) {
        return  weightFunction->accept(*this);
    } else {
        return Edge::weight->accept(*this);
    }
}

double Edge::weigth() {
    return this->weigth(Edge::weight);
}


OGRGeometry *Edge::getShape() {
    if (this->from->getPolygon()->touches(*this->to->getPolygon())) {
        return nullptr;
    }

    /* Get Line connecting the centers of the two polygons representing the settlement */
    auto centerFrom = this->from->getPolygon()->centroid();
    auto centerTo = this->to->getPolygon()->centroid();
    Line pathToFrom = Line(centerFrom, centerTo);

    /* Find intersection point of polygon <from> and shortest line between to and from, which is the farest away from the center of <from>*/
    Line crossFrom = Line();
    auto intersectionFrom = Vec2D();
    double distanceToCenterFrom = 0;
    for (auto &l: this->from->getPolygon()->getLines()) {
        if (l.intersectionOnLine(pathToFrom) and pathToFrom.intersectionOnLine(l)) {
            Vec2D intersection = l.intersection(pathToFrom);
            if(intersection.distance(centerFrom) > distanceToCenterFrom) {
                distanceToCenterFrom = intersection.distance(centerFrom);
                intersectionFrom = intersection;
                crossFrom = l;
            }
        }
    }

    /* Analog for polygon <to>*/
    Line crossTo = Line();
    auto intersectionTo = Vec2D();
    double distanceToCenterTo = 0;
    for (auto &l: this->to->getPolygon()->getLines()) {
        if (l.intersectionOnLine(pathToFrom) and pathToFrom.intersectionOnLine(l)) {
            Vec2D intersection = l.intersection(pathToFrom);
            if (intersection.distance(centerTo) > distanceToCenterTo) {
                distanceToCenterTo = intersection.distance(centerTo);
                intersectionTo = intersection;
                crossTo = l;
            }
        }
    }

    if (not crossFrom.valid() or not crossTo.valid()) {
        return nullptr;
    }
    /* Apply width to the line for the visualization (see Thesis)*/
    double WIDTH = 0.000001;
    auto orthogonalFrom = crossFrom.directionNormalized();
    auto orthogonalTo = crossTo.directionNormalized();
    auto center = centerFrom + (pathToFrom.direction() / 2);
    auto p1 = (intersectionFrom + (orthogonalFrom * WIDTH / 2));
    auto p2 = (intersectionFrom - (orthogonalFrom * WIDTH / 2));
    auto p3 = (intersectionTo + (orthogonalTo * WIDTH / 2));
    auto p4 = (intersectionTo - (orthogonalTo * WIDTH / 2));

    /*Comparator to sort the points clockwise*/
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






