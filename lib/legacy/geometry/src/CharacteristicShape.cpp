//
// Created by grube on 14.01.2022.
//

#include "CharacteristicShape.h"
#include "Ring.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <queue>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_2<K> Triangulation;
typedef Triangulation::Point Point;


std::vector<Vec2D> CharacteristicShape::charactersticConcaveHull(std::vector<Vec2D> &points, double lambda) {
    //clean up points
    std::unordered_set<Vec2D,Vec2D::Hash> ps;
    /* Get most left,right,upper and lower point */
    auto lower = points[0];
    auto left = points[0];
    auto right = points[0];
    auto up = points[0];

    double smallestDistance = MAXFLOAT;
    for (int i = 0; i< points.size(); i++) {
        auto p = points[i];
        ps.insert(p);
        if (i < points.size() - 1 and p.distance(points[i + 1]) < smallestDistance) {
            smallestDistance = p.distance(points[i + 1]);
        }
        if (p.x < left.x) {
            left = p;
        }
        if (p.y < lower.y) {
            lower = p;
        }
        if (p.x > right.x) {
            right = p;
        }
        if (p.y > up.y) {
            up = p;
        }
    }
    std::vector<Point> adaptToCGALPoint;
    adaptToCGALPoint.reserve(points.size());
    for (auto &p: ps) {
        adaptToCGALPoint.emplace_back(p.x, p.y);
    }
    Triangulation T;
    T.insert(adaptToCGALPoint.begin(), adaptToCGALPoint.end());
    std::vector<Line> lines;
    /*Get lines from CGAL triangulation */
    for (auto &e: T.finite_edges()) {
        auto p1 = e.first->vertex((e.second + 1) % 3)->point();
        auto p2 = e.first->vertex((e.second + 2) % 3)->point();
        double p1X = CGAL::to_double(p1.x());
        double p1Y = CGAL::to_double(p1.y());
        double p2X = CGAL::to_double(p2.x());
        double p2Y = CGAL::to_double(p2.y());
        lines.emplace_back(Vec2D(p1X, p1Y), Vec2D(p2X, p2Y));
    }
    auto largestDistance = std::max(up.distance(lower), left.distance(right));
    //adjust edgeLength that will be tested for replacement by the algo according to maximum / minimum distances between the points and the lambda factor
    double edgeLength = lambda *(largestDistance - smallestDistance) + smallestDistance;
    if (not T.is_valid()) {
        edgeLength = MAXFLOAT; //compute convex hull when triangulation fails
    }
    // https://www.researchgate.net/publication/220600217_Efficient_generation_of_simple_polygons_for_characterizing_the_shape_of_a_set_of_points_in_the_plane
    auto map = CombinatorialMap(lines); //https://en.wikipedia.org/wiki/Combinatorial_map
    auto edges = map.getEdges();
    auto cmp = [](const DartEdge &e1, const DartEdge & e2){
        return e1.length() < e2.length(); //priority queue -> biggest element is considered first, i.e. longest edge to be considered first
    };
    std::unordered_map<Vertex, bool, Vertex::Hash> isBoundaryVertex;
    /* Initialize boundary vertices*/
    for (auto &v: map.getVertices()) {
        isBoundaryVertex.insert(std::make_pair(v, false));
    }
    /* Find boundary edges + add to Queue and set boundary vertices*/
    auto boundary = std::priority_queue<DartEdge,std::vector<DartEdge>, decltype(cmp)>(cmp);
    for (auto &edge: edges) {
        if (map.boundaryEdge(edge)) {
            boundary.push(edge);
            isBoundaryVertex.at(CombinatorialMap::vertex(edge.d1)) = true;
            isBoundaryVertex.at(CombinatorialMap::vertex(edge.d2)) = true;
        }
    }
    /* Try to replace longest boundary edge until none is left*/
    while (not boundary.empty()) {
        auto e = boundary.top();
        boundary.pop();
        if (e.length() > edgeLength and map.boundaryEdge(e)) {
            auto revealedVertex = CombinatorialMap::vertex(map.alpha(map.reveal(e.d1))); // e.d2 is also valid
            if (not isBoundaryVertex.at(revealedVertex)) {
                auto reveal1 = map.reveal(e.d1);
                auto reveal2 = map.reveal(e.d2);
                isBoundaryVertex.at(revealedVertex) = true;
                map.removeEdge(e);
                auto e1 = map.edge(reveal1); //first inner edge to be revealed by removing boundary edge e
                auto e2 = map.edge(reveal2); //second inner edge to be revealed by removing boundary edge e
                boundary.push(e1); // add new boundary edges to the queue, in order for them to be considered later
                boundary.push(e2);
            }
        }
    }
    auto polygonBoundaryEdges = map.getBoundaryEdges();
    auto boundaryEdgeCount = polygonBoundaryEdges.size();
    auto polygonPoints = std::vector<Vec2D>();


    /* Find ring in unsorted edges. Starting from a random starting edge: Find the next edge, that extends the curve (this is the edge that has exactly one point in common with the current)*/
    auto first = polygonBoundaryEdges[0].getFixpoint();
    auto current = first;
    polygonBoundaryEdges.erase(polygonBoundaryEdges.begin());
    while (polygonPoints.size() < boundaryEdgeCount) {
        polygonPoints.push_back(current);
        for (auto itL = polygonBoundaryEdges.begin(); itL != polygonBoundaryEdges.end();) {
            auto l = *itL;
            if (l.getFixpoint() == current) {
                current = l.getReferencePoint();
                polygonBoundaryEdges.erase(itL);
                break;
            } else if (l.getReferencePoint() == current) {
                current = l.getFixpoint();
                polygonBoundaryEdges.erase(itL);
                break;
            }
            ++itL;
        }
    }
    polygonPoints.push_back(first);
    return polygonPoints;
}

//CombinatorialMap CharacteristicShape::delaunayTriangulation(std::unordered_set<Vec2D,Vec2D::Hash> &points) {
//    std::vector<Point> adaptToCGALPoint;
//    adaptToCGALPoint.reserve(points.size());
//    for (auto &p: points) {
//        adaptToCGALPoint.emplace_back(p.x, p.y);
//    }
//    Triangulation T;
//    T.insert(adaptToCGALPoint.begin(), adaptToCGALPoint.end());
//    std::vector<Line> edges;
//    for (auto &e: T.finite_edges()) {
//        auto p1 = e.first->vertex((e.second + 1) % 3)->point();
//        auto p2 = e.first->vertex((e.second + 2) % 3)->point();
//        double p1X = CGAL::to_double(p1.x());
//        double p1Y = CGAL::to_double(p1.y());
//        double p2X = CGAL::to_double(p2.x());
//        double p2Y = CGAL::to_double(p2.y());
//        edges.emplace_back(Vec2D(p1X, p1Y), Vec2D(p2X, p2Y));
//    }
//    return CombinatorialMap(edges);
//}
