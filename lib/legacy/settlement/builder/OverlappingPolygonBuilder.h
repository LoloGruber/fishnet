#ifndef OverlappingPolygonBuilder_H
#define OverlappingPolygonBuilder_H
#include "ISettlementBuilder.h"
class OverlappingPolygonBuilder:public ISettlementBuilder {
public:
    OverlappingPolygonBuilder(std::unique_ptr<const Polygon> & settlementArea):ISettlementBuilder(settlementArea){};
    void setImperviousness(const std::unordered_map<std::unique_ptr<Polygon>,double> & impMap) override;
    void setPopulation(const std::unordered_map<std::unique_ptr<Polygon>,int> &popMap) override;
};
#endif