#ifndef ISettlementBuilder_H
#define ISettlementBuilder_H
#include "../Settlement.h"
#include <unordered_map>
class ISettlementBuilder : public Settlement {
public:
    ISettlementBuilder(std::unique_ptr<const Polygon> & polygon):Settlement(polygon){};
    virtual void setImperviousness(const std::unordered_map<std::unique_ptr<Polygon>,double> & impMap)=0;
    virtual void setPopulation(const std::unordered_map<std::unique_ptr<Polygon>,int> &popMap)=0;
    virtual ~ISettlementBuilder(){};

    std::unique_ptr<Settlement> toSettlement(){
        return std::make_unique<Settlement>(this->getPolygon(),this->imperviousness,this->population);
    }
};
#endif