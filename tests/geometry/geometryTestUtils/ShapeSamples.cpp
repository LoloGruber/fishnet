#include "ShapeSamples.h"


Ring<double> LinearRingSamples::aaBB(Vec2D<double>  topLeft, Vec2D<double>   botRight)  {
    if(topLeft == botRight) 
        throw std::invalid_argument("Points are coinciding");
    if(topLeft.x == botRight.x || topLeft.y == botRight.y)
        throw std::invalid_argument("Points do not form an area");
    return Ring(std::initializer_list<Vec2D<double>>{topLeft,Vec2D(botRight.x,topLeft.y),botRight,Vec2D(topLeft.x,botRight.y)});
}

Ring<double> LinearRingSamples::triangle(Vec2D<double> a, Vec2D<double> b, Vec2D<double> c) {
    if(Line(a,b).contains(c)) throw std::invalid_argument("Points are collinear");
    return Ring(std::initializer_list<Vec2D<double>>{a,b,c});
}

Ring<double> LinearRingSamples::aaRhombus(Vec2D<double> center, double radius){
    return static_cast<Ring<double>>(SimplePolygonSamples::aaRhombus(center,radius));
}

static std::vector<Vec2D<double>> getPointsOfComplexRing(){
        auto points = std::vector<Vec2D<double>>();
        points.emplace_back(0,4);
        points.emplace_back(2,3);
        points.emplace_back(2,2);
        points.emplace_back(4,2);
        points.emplace_back(2,0);
        points.emplace_back(-2,-1);
        points.emplace_back(0,1);
        points.emplace_back(-3,2);
        points.emplace_back(0,4);
        return points;
}
const  Ring<double> LinearRingSamples::COMPLEX_RING = Ring(getPointsOfComplexRing());

const SimplePolygon<double> SimplePolygonSamples::COMPLEX_BOUNDARY = SimplePolygon(LinearRingSamples::COMPLEX_RING);

SimplePolygon<double> SimplePolygonSamples::aaBB(Vec2D<double> topLeft, Vec2D<double> botRight){
    return SimplePolygon(LinearRingSamples::aaBB(topLeft,botRight));
}

SimplePolygon<double> SimplePolygonSamples::triangle(Vec2D<double> a, Vec2D<double> b, Vec2D<double> c){
    return SimplePolygon<double>(LinearRingSamples::triangle(a,b,c));
}

SimplePolygon<double> SimplePolygonSamples::aaRhombus(Vec2D<double> center, double radius){
    if(radius <= 0)
        throw std::invalid_argument("Radius has to be > 0");
    return SimplePolygon<double>({
        {center.x,center.y+radius},
        {center.x+radius,center.y},
        {center.x,center.y - radius},
        {center.x-radius,center.y}
    });
}

