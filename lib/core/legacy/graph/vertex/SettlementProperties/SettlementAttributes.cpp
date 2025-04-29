//
// Created by grube on 06.01.2022.
//

#include <stdexcept>
#include "SettlementAttributes.h"
const char * SettlementAttributes::popString = static_cast<const char *> ("wsf_pop");
const char *SettlementAttributes::impString = static_cast<const char *> ("wsf_imp");
const char *SettlementAttributes::areaString = static_cast<const char *> ("wsf_area");
const char *SettlementAttributes::entityString = static_cast<const char *> ("settlement");

SettlementAttributes::SettlementAttributes(double area, double imperviousness, int population) : area(area),imperviousness(imperviousness),population(population) {
    if (not isValidArea(area)) {
        throw std::invalid_argument("Area has to be greater than 0");
    }
    if (isValidPopulation(population) and isValidImperviousness(imperviousness)) {
        this->type = WSFSettlementType::FULL;
    } else if (isValidImperviousness(imperviousness)) {
        this->type = WSFSettlementType::IMPERVIOUSNESS;
    } else if (isValidPopulation(population)) {
        this->type = WSFSettlementType::POPULATION;
    } else {
        this->type = WSFSettlementType::BASE;
    }
}

bool SettlementAttributes::isValidArea(double area) {
    return area > 0.0;
}

bool SettlementAttributes::isValidImperviousness(double imp) {
    return imp >= 0.0 and imp <= 1.0;
}

bool SettlementAttributes::isValidPopulation(int pop) {
    return pop > 0;
}

double SettlementAttributes::getArea() const {
    return this->area;
}

double SettlementAttributes::getImperviousness() const {
    return this->imperviousness;
}

int SettlementAttributes::getPopulation() const {
    return this->population;
}

WSFSettlementType SettlementAttributes::getType() const {
    return this->type;
}

void SettlementAttributes::createFields(OGRLayer * layer) {
    auto fields = std::list<OGRFieldDefn *>();
    fields.push_back(new OGRFieldDefn(popString, OFTInteger));
    auto i = new OGRFieldDefn(impString, OFTReal);
    i->SetPrecision(4);
    fields.push_back(i);
    auto a = new OGRFieldDefn(areaString, OFTReal);
    a->SetPrecision(4);
    fields.push_back(a);

    for (auto &def: fields) {
        layer->CreateField(def);
        delete (def);
    }
}
/**
 * This method wont behave as expected if the fields are not initialized
 * @param feature
 */
void SettlementAttributes::setFields(OGRFeature *feature) const {
    feature->SetField(EntityField::entityFieldName, entityString);
    feature->SetField(areaString, area);
    feature->SetField(impString, imperviousness);
    feature->SetField(popString, population);
}