#include "Settlement.h"

Settlement::Settlement(std::unique_ptr<const Polygon> & polygon, double imperviousness,int population){
    this->polygon = std::move(polygon);
    this->population = population;
    this->imperviousness = imperviousness;
}

// const std::unique_ptr<Polygon> & Settlement::getPolygon() const{
//     return this->polygon;
// }

const double Settlement::getArea() const {
    return this->polygon->area();
}

const std::optional<int> Settlement::getPopulation() const {
    if(isValidPopulation(this->population)) return std::make_optional<int>(this->population);
    return std::nullopt;
}

const std::optional<double> Settlement::getImperviousness() const {
    if(isValidImperviousness(this->imperviousness)) return std::make_optional<double>(this->imperviousness);
    return std::nullopt;
}


bool Settlement::isValidArea(double area) {
    return area > 0.0;
}

bool Settlement::isValidImperviousness(double imp) {
    return imp >= 0.0 and imp <= 1.0;
}

bool Settlement::isValidPopulation(int pop) {
    return pop > 0;
}
