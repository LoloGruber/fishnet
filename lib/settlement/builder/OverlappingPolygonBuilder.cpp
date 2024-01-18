#include "OverlappingPolygonBuilder.h"

void OverlappingPolygonBuilder::setImperviousness(const std::unordered_map<std::unique_ptr<Polygon>,double> & impMap){
    double imp = -1;
    for(auto it = impMap.begin(); it!= impMap.end(); ++it){
        if(this->getPolygon()->contains(*it->first)) {
            if(imp==-1) {
                imp = it->second;
            } else {
                imp = imp * ((this->getPolygon()->area() - it->first->area()) / this->getPolygon()->area()) 
                + (it->first->area()/this->getPolygon()->area())*it->second;
            }
        }
    }
    this->imperviousness = imp;
}

void OverlappingPolygonBuilder::setPopulation(const std::unordered_map<std::unique_ptr<Polygon>,int> & popMap){
    int pop = 0;
    for (auto it=popMap.begin(); it!=popMap.end(); ++it){
        if(this->getPolygon()->contains(*it->first)) {
            pop+=it->second;
        }
    }
    this->population = pop;
}