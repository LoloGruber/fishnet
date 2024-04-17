#ifndef Settlement_H
#define Settlement_H
#include <memory>
#include <list>
#include "geometry/Polygon.h"
class Settlement {
private:
    std::unique_ptr<const Polygon> polygon;
protected:
    double imperviousness;
    int population;
public:
    Settlement(std::unique_ptr<const Polygon> & polygon, double imperviousness=-1.0,int population = -1);
    [[nodiscard]] std::unique_ptr<const Polygon> & getPolygon() const;
    [[nodiscard]] const double getArea() const;
    [[nodiscard]] const std::optional<int> getPopulation() const;
    [[nodiscard]] const std::optional<double> getImperviousness() const;

    static bool isValidArea(double area);
    static bool isValidImperviousness(double im);
    static bool isValidPopulation(int pop);

    
};
#endif