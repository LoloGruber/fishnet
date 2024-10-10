#pragma once
#include <fishnet/DegreeCentrality.hpp>
#include "CentralityMeasureType.hpp"

struct DegreeCentralityMeasure {
    static CentralityMeasureType type() noexcept{
        return CentralityMeasureType::DegreeCentrality;
    }
    template<fishnet::graph::Graph GraphType,typename GeometryType>
    void operator()(const GraphType & source, fishnet::VectorLayer<GeometryType> & layer,std::unordered_map<size_t,fishnet::Feature<GeometryType>> & result) const  {
        fishnet::graph::DegreeCentrality degCent;
        auto field = layer.addSizeField("DegreeCent");
        if (not field){
            throw std::runtime_error("Could not create field \"DegreeCent\" for Degree Centrality Measure");
        }
        for(auto && [node,degCentrality] : degCent(source)){
            result.at(node.key()).addAttribute(field.value(),degCentrality);
        }
    }
};