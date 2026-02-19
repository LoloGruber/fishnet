#pragma once
#include <functional>

class AreaClass{
public:
    uint8_t id;
    double upperAreaLimit;
    bool operator==(const AreaClass & other) const {
        return this->id == other.id;
    }

    bool operator<(const AreaClass & other) const {
        return this->upperAreaLimit < other.upperAreaLimit;
    }
};

namespace std{
    template<>
    struct hash<AreaClass>{
        size_t operator()(const AreaClass & areaClass) const {
            return areaClass.id;
        }
    };
}