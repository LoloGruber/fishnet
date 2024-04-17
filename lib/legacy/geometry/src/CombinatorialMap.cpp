//
// Created by grube on 14.01.2022.
//

#include <iostream>
#include "CombinatorialMap.h"
#include "DartEdge.h"

Dart CombinatorialMap::alpha(const Dart &dart) {
/*    if (not alphaMap.contains(dart)) {

    }*/
    return alphaMap.at(dart);
}

Dart CombinatorialMap::sigma(const Dart &dart) {
    auto neighbors = sigmaPerVertex.at(vertex(dart));
    auto position = neighbors.find(dart);
    if ((++position) == neighbors.end()) {
        return *neighbors.begin();
    } else {
        return *position;
    }
}

Dart CombinatorialMap::reveal(const Dart &dart) {
    if (alpha(sigma(alpha(sigma(alpha(sigma(dart)))))) == dart) {
        return sigma(dart);
    } else {
        return alpha(sigma(alpha(sigma(alpha(dart)))));
    }
}


Vertex CombinatorialMap::vertex(const Dart &dart) {
    return dart.from;
}

CombinatorialMap::CombinatorialMap(std::vector<Line> &lines) {
    bool printed = false;
    for (auto &l: lines) {
        if (l.valid()) {
/*  Edge Case: NOT SOLVED
*          if (not printed and l.getReferencePoint() == (Vec2D(-7.7970173399084137, 33.454967979895379)) or
                l.getFixpoint() == (Vec2D(-7.7970173399084137, 33.454967979895379))) {
                printed = true;
                OGRMultiLineString ogrlines = OGRMultiLineString();
                for (auto &line: lines) {
                    auto ogrline = new OGRLineString();
                    ogrline->addPoint(l.getFixpoint().toOGRPoint());
                    ogrline->addPoint(l.getReferencePoint().toOGRPoint());
                    ogrlines.addGeometryDirectly(ogrline);
                }
                auto output = ogrlines.exportToJson();
                std::cout << output << std::endl;

            }*/
            // Create half edges
            Vertex v1 = Vertex(l.getFixpoint(), VERTEXID++);
            Vertex v2 = Vertex(l.getReferencePoint(), VERTEXID++);
            if (points.contains(v1.position)) {
                v1 = Vertex(v1.position, points.at(v1.position).id);
            }
            if (points.contains(v2.position)) {
                v2 = Vertex(v2.position, points.at(v2.position).id);
            }
            points.insert({v1.position, v1});
            points.insert({v2.position, v2});

            /* Each line is deconstructed into two half edges -> identified by their ID*/
            Dart d1 = Dart(v1, v2, ID++);
            Dart d2 = Dart(v2, v1, ID++);

            /* Create new set of darts per vertex, if the map does not contain an entry for v1 */
            if (sigmaPerVertex.find(v1) == sigmaPerVertex.end()) {
                sigmaPerVertex.insert(std::make_pair(v1, std::set<Dart>()));
            }
            sigmaPerVertex.at(v1).insert(d1);

            /* Create new set of darts per vertex, if the map does not contain an entry for v2 */
            if (sigmaPerVertex.find(v2) == sigmaPerVertex.end()) {
                sigmaPerVertex.insert(std::make_pair(v2, std::set<Dart>()));
            }
            sigmaPerVertex.at(v2).insert(d2);
            D.insert(d1);
            D.insert(d2);
            alphaMap.insert(std::make_pair(d1, d2));
            alphaMap.insert(std::make_pair(d2, d1));
        }
    }
}

DartEdge CombinatorialMap::edge(const Dart &dart) {
    return {dart, alpha(dart)};
}

std::unordered_set<DartEdge,DartEdge::Hash,DartEdge::Equal> CombinatorialMap::getEdges() {
    std::unordered_set<DartEdge,DartEdge::Hash,DartEdge::Equal> edgeSet;
    for (auto &d: D) {
        bool insert = true;
        /*Prevent duplicate insertions, this was added to rule out possible errors with map*/
        for (auto &e: edgeSet) {
            if (e.d1 == d or e.d2 == d) {
                insert = false;
            }
        }
        if (insert) {
            edgeSet.insert(edge(d));
        }

    }
    return edgeSet;
}

std::unordered_set<Vertex, Vertex::Hash> CombinatorialMap::getVertices(){
    std::unordered_set<Vertex, Vertex::Hash> vertices;
    for (auto &v: sigmaPerVertex) {
        vertices.insert(v.first);
    }
    return vertices;
}

bool CombinatorialMap::boundaryEdge(const DartEdge &edge) {
    return not (alpha(sigma(alpha(sigma(alpha(sigma(edge.d1)))))) == edge.d1 and
           alpha(sigma(alpha(sigma(alpha(sigma(edge.d2)))))) == edge.d2);
}


std::vector<Line> CombinatorialMap::getBoundaryEdges() {
    auto boundary = std::vector<Line>();
    for (auto &e: getEdges()) {
        if (boundaryEdge(e)) {
            bool insert = true;
            /* Prevent duplicate insertion of line*/
            for (auto &l: boundary) {
                if (Line(e.d1.from.position, e.d1.to.position).equalNoOrientation(l)) {
                    insert = false;
                    break;
                }
            }
            if (insert) {
                boundary.emplace_back(e.d1.from.position, e.d1.to.position);
            }
        }
    }
    return boundary;
}

void CombinatorialMap::removeEdge(const DartEdge &edge) {
    auto d1 = edge.d1;
    auto d2 = edge.d2;
    D.erase(d1);
    D.erase(d2);
    sigmaPerVertex.at(vertex(d1)).erase(d1);
    sigmaPerVertex.at(vertex(d2)).erase(d2);
    alphaMap.erase(d1);
    alphaMap.erase(d2);
}

bool CombinatorialMap::exists(const DartEdge &edge) {
    return alphaMap.find(edge.d1) != alphaMap.end() and alphaMap.find(edge.d2) != alphaMap.end();
}


