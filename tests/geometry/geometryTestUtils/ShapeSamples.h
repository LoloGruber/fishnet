#pragma once
#include "Ring.hpp"
#include "SimplePolygon.hpp"
#include <memory>
using namespace fishnet::geometry;



class LinearRingSamples{
public:
const static Ring<double> COMPLEX_RING;
static Ring<double> aaBB(Vec2D<double> topLeft, Vec2D<double>  bottomRight);
static Ring<double> triangle(Vec2D<double> a, Vec2D<double> b, Vec2D<double> c);
static Ring<double> aaRhombus(Vec2D<double> center, double radius);
};

class SimplePolygonSamples {
public:
static SimplePolygon<double> aaBB(Vec2D<double> topLeft, Vec2D<double> botRight);
static SimplePolygon<double> triangle(Vec2D<double> a, Vec2D<double> b, Vec2D<double> c);
static SimplePolygon<double> aaRhombus(Vec2D<double> center, double radius);
const static SimplePolygon<double> COMPLEX_BOUNDARY; 
};




