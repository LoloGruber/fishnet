//
// Created by grube on 08.01.2022.
//

#include "TwoShortestLinesMergeStrategy.h"
#include "utils/Geometry/Line.h"
#include "utils/Geometry/Ring.h"

bool TwoShortestLinesMergeStrategy::contains(std::vector<Vec2D> &points, Vec2D &point) {
    for (auto &p: points) {
        if (p == point) {
            return true;
        }
    }
    return false;
}

void TwoShortestLinesMergeStrategy::insert(std::vector<Vec2D> &points, Vec2D &point) {
    if (not contains(points, point)) {
        points.push_back(point);
    }
}

std::unique_ptr<Polygon> TwoShortestLinesMergeStrategy::merge(const Polygon &u, const Polygon & v) {
    auto ps = u.getPoints();
    auto qs = v.getPoints();
    /*Remove duplicated points from the larger point list qs or ps*/
    if (ps.size() > qs.size()) {
        auto ring = Ring(qs, v.getLines());
        for(auto it=ps.begin(); it != ps.end();){
            if (ring.contains(*it)) {
                ps.erase(it);
            } else {
                ++it;
            }
        }
    }else{
        auto ring = Ring(ps, u.getLines());
        for(auto it=qs.begin(); it != qs.end();){
            if (ring.contains(*it)) {
                qs.erase(it);
            } else {
                ++it;
            }
        }
    }

    /* ps has too few points -> return Polygon of qs*/
    if (ps.size() < 4) {
        auto p = Polygon::create(qs);
        return p;
    }
    /* qs has too few points -> return Polygon of ps*/
    if (qs.size() < 4) {
        auto p = Polygon::create(ps);
        return p;
    }

    /* Get all exterior lines of u and v -> store in "allLines"*/
    double minDistance = MAXFLOAT;
    std::vector<Line> allLines;


    for (int i= 0; i< ps.size()-1; i++) {
        allLines.emplace_back(ps[i],ps[i+1]);
    }
    for (int i= 0; i< qs.size()-1; i++) {
        allLines.emplace_back(qs[i],qs[i+1]);
    }

    Line shortest = Line();
    auto indexShortest = std::make_pair<long, long>(-1,-1); //pair storing the index of the points for the shortest line: <IndexInPS, IndexInQS>

    /* Find the shortest line connecting p and q*/
    for (auto p = ps.begin(); p != ps.end(); p++) {
        for (auto q = qs.begin(); q != qs.end(); q++) {
            Line current = Line(*p, *q);
            if (current.length() < minDistance) {
                for (auto &l: allLines) {
                    /* Verify that the current line does NOT intersect any of the boundary lines of u or v -> no self-intersections allowed!*/
                    if(not current.intersectionOnLine(l)) {
                        shortest = current;
                        indexShortest = std::make_pair(std::distance(ps.begin(), p), std::distance(qs.begin(), q));
//                            indexShortestFixpoint =  std::distance(ps.begin(), p);
//                            indexShortestReference = std::distance(qs.begin(), q);
                        minDistance = current.length();
                    }
                }
            }

        }
    }
    Line secondShortest = Line();
    auto indexSecondShortest = std::make_pair<long, long>(-1,-1);
    minDistance = MAXFLOAT;
    Vec2D lowerLeftPoint = ps[0];
    long indexLowerLeftPoint = 0;
    bool lowerLeftPointInP = true; //specifies if most lower left point is in points list of u (ps)

    /* Analogously find the second shortest line and find most lower left in the same loop (save computing time)*/
    for (auto p = ps.begin(); p != ps.end(); p++) {
        if (p->x < lowerLeftPoint.x and p->y <= lowerLeftPoint.y or p->x <= lowerLeftPoint.x and
            p->y < lowerLeftPoint.y) {
            lowerLeftPoint = *p;
            indexLowerLeftPoint = std::distance(ps.begin(), p);
            lowerLeftPointInP = true;
        }
        for (auto q = qs.begin(); q != qs.end(); q++) {
            /* Second shortest line is not allowed to have any common points with the shortest line*/
            if (*p != shortest.getFixpoint() and *p != shortest.getReferencePoint() and
                *q != shortest.getFixpoint() and *q != shortest.getReferencePoint()) {
                Line current = Line(*p, *q);
                if (current.length() < minDistance) {
                    if (not shortest.intersectionOnLine(current)) {
                        for (auto &l: allLines) {
                            /* Verify that the current line does NOT intersect any of the boundary lines of u or v -> no self-intersections allowed!*/
                            if(not current.intersectionOnLine(l)) {
                                secondShortest = current;
                                indexSecondShortest = std::make_pair(std::distance(ps.begin(), p),
                                                                     std::distance(qs.begin(), q));
                                minDistance = current.length();
                            }
                        }
                    }
                }
            }
            if (q->x < lowerLeftPoint.x and q->y <= lowerLeftPoint.y or q->x <= lowerLeftPoint.x and
                q->y < lowerLeftPoint.y) {
                lowerLeftPoint = *q;
                indexLowerLeftPoint = std::distance(qs.begin(), q);
                lowerLeftPointInP = false;
            }
        }
    }
    if (indexShortest.first == -1 or indexSecondShortest.first == -1) {
        throw std::invalid_argument("contraction not possible due to insufficient amount of lines");
    }

    /* Walk exterior lines starting from most lower left point and switch to other polygon when reaching the new connections <shortest> or <second shortest> line*/
    std::vector<Vec2D> points;
    if (lowerLeftPointInP) {
        long indexP = indexLowerLeftPoint;
        long indexQ;
        while (true) {
            if (indexP == indexShortest.first) {
                indexQ = indexShortest.second;
                break;
            }
            if (indexP == indexSecondShortest.first) {
                indexQ = indexSecondShortest.second;
                break;
            }
            insert(points, ps[indexP]);
            indexP = (indexP + 1) % (long) ps.size();
        }
        insert(points, ps[indexP]);
        insert(points, qs[indexQ]);
        indexQ = (indexQ + 1) % (long) qs.size();

        while (true) {
            if (indexQ == indexShortest.second) {
                indexP = indexShortest.first;
                break;
            }
            if (indexQ == indexSecondShortest.second) {
                indexP = indexSecondShortest.first;
                break;
            }
            insert(points, qs[indexQ]);
            indexQ = (indexQ + 1) % (long) qs.size();
        }
        insert(points, qs[indexQ]);
        while (indexP != indexLowerLeftPoint) {
            insert(points, ps[indexP]);
            indexP = (indexP + 1) % (long) ps.size();
        }
        points.push_back(ps[indexLowerLeftPoint]);
    } else{ //lower left point is in q, start walk in q
        long indexQ = indexLowerLeftPoint;
        long indexP;
        while (true) {
            if (indexQ == indexShortest.second) {
                indexP = indexShortest.first;
                break;
            }
            if (indexQ == indexSecondShortest.second) {
                indexP = indexSecondShortest.first;
                break;
            }
            insert(points, qs[indexQ]);
            indexQ = (indexQ + 1) % (long) qs.size();
        }
        insert(points, qs[indexQ]);
        insert(points, ps[indexP]);
        indexP = (indexP + 1) % (long) ps.size();
        while (true) {
            if (indexP == indexShortest.first) {
                indexQ = indexShortest.second;
                break;
            }
            if (indexP == indexSecondShortest.first) {
                indexQ = indexSecondShortest.second;
                break;
            }
            insert(points, ps[indexP]);
            indexP = (indexP + 1) % (long) ps.size();
        }
        insert(points, ps[indexP]);
        while (indexQ != indexLowerLeftPoint) {
            insert(points, qs[indexQ]);
            indexQ = (indexQ + 1) % (long) qs.size();
        }
        points.push_back(qs[indexLowerLeftPoint]);
    }
    auto *ring = new OGRLinearRing();
    for (auto &vec: points) {
        ring->addPoint(vec.toOGRPoint()); // add points to ring
    }
    auto polygonOGR = OGRPolygon();
    polygonOGR.addRing(ring);
    return std::make_unique<Polygon>(polygonOGR);
}
