#pragma once
#include <fishnet/Graph.hpp>
#include <fishnet/Feature.hpp>
#include "CentralityMeasureType.hpp"

struct MeanLocalSignificance{
    static CentralityMeasureType type() noexcept {
        return CentralityMeasureType::MeanLocalSignificance;
    }

    template<fishnet::graph::Graph GraphType, typename GeometryType>
    void operator()(const GraphType & source, fishnet::VectorLayer<GeometryType> & layer, std::unordered_map<size_t,fishnet::Feature<GeometryType>> & result) const {
        auto field = layer.addDoubleField("MeanLocSig");
        if(not field){
            throw std::runtime_error("Could not create field \"MeanLocSig\"");
        }
        for(const auto & node : source.getNodes()) {
            double accLocalSig = 0;
            int count = 0;
            for(const auto & neighbour : source.getNeighbours(node)){
                accLocalSig+= (node.area() * neighbour.area()) / pow(node.distance(neighbour),2);
                count++;
            }
            double meanLocalSig = count==0?0.0:accLocalSig / count;
            result.at(node.key()).addAttribute(field.value(),meanLocalSig);
        }
    }
};
