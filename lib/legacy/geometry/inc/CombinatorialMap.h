//
// Created by grube on 14.01.2022.
//

#ifndef WSF_NETWORK_COMBINATORIALMAP_H
#define WSF_NETWORK_COMBINATORIALMAP_H


#include <vector>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include "Dart.h"
#include "Line.h"
#include "DartEdge.h"

//https://en.wikipedia.org/wiki/Combinatorial_map
class CombinatorialMap {
private:
    std::size_t ID =0;
    std::size_t VERTEXID = 0;
    std::unordered_set<Dart,Dart::Hash> D;
    std::unordered_map<Dart,Dart,Dart::Hash> alphaMap;
    std::unordered_map<Vertex,std::set<Dart>,Vertex::Hash> sigmaPerVertex;
    std::unordered_map<Vec2D,Vertex,Vec2D::Hash> points;
public:

    explicit CombinatorialMap(std::vector<Line> &lines);

    void removeEdge(const DartEdge & edge);
    /**
     * Get opposite half-edge / dart
     * @param dart
     * @return
     */
    Dart alpha(const Dart & dart);

    /**
     * Get next half-edge at vertex in ccw direction
     * @param dart
     * @return
     */
    Dart sigma(const Dart &dart);

    /**
     * yield that dart that will be revealed on the boundary if the given dart would be remove
     * @param dart
     * @return
     */
    Dart reveal(const Dart &dart);

    /**
     * Get vertex of dart;
     * @param dart
     * @return
     */
    static Vertex vertex(const Dart &dart);

    /**
     *
     * @param dart
     * @return edge corresponding to the given dart
     */
    DartEdge edge(const Dart &dart);

    std::unordered_set<DartEdge,DartEdge::Hash,DartEdge::Equal> getEdges();

    std::unordered_set<Vertex, Vertex::Hash> getVertices();

    /**
     * Get the edges (as Line objects) that form the exterior ring of the triangulation
     * @return
     */
    std::vector<Line> getBoundaryEdges();

    /**
     *
     * @param edge
     * @return whether edge is boundary edge
     */
    bool boundaryEdge(const DartEdge & edge);

    bool exists(const DartEdge & edge);



};


#endif //WSF_NETWORK_COMBINATORIALMAP_H
