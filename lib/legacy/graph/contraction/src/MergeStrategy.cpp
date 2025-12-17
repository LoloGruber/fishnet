//
// Created by grube on 15.01.2022.
//
#include "MergeStrategy.h"

std::shared_ptr<Settlement> MergeStrategy::merge(const Settlement &s1, const Settlement &s2) {
    auto attributeMerge = this->attributeMergeStrategy->combine(*s1.getAttributes(), *s2.getAttributes()); //merge attributes with specified strategy
    auto polygonMerge = this->polygonMergeStrategy->merge(*s1.getPolygon(), *s2.getPolygon()); //merge polygons with specified strategy
    return std::make_unique<Settlement>(polygonMerge, attributeMerge);
}
