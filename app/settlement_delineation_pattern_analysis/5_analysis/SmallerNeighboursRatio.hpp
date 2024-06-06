#pragma once
#include <fishnet/Graph.hpp>
#include "CentralityMeasureType.hpp"

struct SmallerNeighboursRatio{
    static CentralityMeasureType type() noexcept {
        return CentralityMeasureType::SmallerNeighboursRatio;
    }

    template<fishnet::graph::Graph GraphType, typename GeometryType>
    void operator()(const GraphType & source, fishnet::VectorLayer<GeometryType> & layer, std::unordered_map<size_t,fishnet::Feature<GeometryType>> & result) const {
        auto field = layer.addDoubleField("SmallNeigh");
        if(not field){
            throw std::runtime_error("Could not create field \"SmallerNei%\"");
        }
        for(const auto & node : source.getNodes()) {
            int smaller = 0;
            int count = 0;
            for(const auto & neighbour : source.getNeighbours(node)){
                if(node.area() > neighbour.area())
                    smaller++;
                count++;
            }
            double smallerNeighboursRatio = count==0?0.0:double(smaller)/double(count);
            result.at(node.key()).addAttribute(field.value(),smallerNeighboursRatio);
        }
    }
};

