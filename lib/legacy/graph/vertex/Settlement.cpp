//
// Created by grube on 25.11.2021.
//

#include "Settlement.h"
#include "graph/vertex/SettlementProperties/WSFSettlementValue.h"

std::shared_ptr<SettlementValue> Settlement::strategy = std::make_shared<WSFSettlementValue>();

Settlement::Settlement(std::unique_ptr<Polygon> &polygon, std::unique_ptr<SettlementAttributes> & attributes) : NetworkEntity(){
    this->polygon = std::move(polygon);
    this->attributes = std::move(attributes);
}

double Settlement::distance(const Settlement &other) const {
    return this->polygon->distance(*other.polygon);
}

const std::unique_ptr<SettlementAttributes> &Settlement::getAttributes() const{
    return this->attributes;
}

double Settlement::value() const{
    if (Settlement::strategy == nullptr) {
        return 0.0;
    }
    return Settlement::strategy->accept(*this);
}

void Settlement::setSettlementValueStrategy(std::shared_ptr<SettlementValue> & settlementValueStrategy){
    Settlement::strategy = settlementValueStrategy;
}

bool Settlement::operator<(const Settlement &other) const{
    return this->value() < other.value();
}

bool Settlement::operator==(const Settlement &other) const {
    return this->polygon == other.polygon;
}
bool Settlement::operator!=(const Settlement &other) const{
    return not(*this == other);
}

const std::unique_ptr<OGRGeometry> & Settlement::getShape() const {
    return this->polygon->getGeometry();
}

const std::unique_ptr<Polygon> &Settlement::getPolygon() const {
    return this->polygon;
}




















