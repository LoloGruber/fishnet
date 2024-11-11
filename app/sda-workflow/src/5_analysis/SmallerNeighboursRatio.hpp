#pragma once
#include <fishnet/Graph.hpp>
#include "CentralityMeasureType.hpp"
#include <execution>

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
        const auto & nodes = source.getNodes();
        std::for_each(std::execution::par, std::ranges::begin(nodes),std::ranges::end(nodes),[&source,&field,&result](const auto & node){
            int smaller = 0;
            int count = 0;
            for(const auto & neighbour : source.getNeighbours(node)){
                if(node.area() > neighbour.area())
                    smaller++;
                count++;
            }
            double smallerNeighboursRatio = count==0?0.0:double(smaller)/double(count);
            result.at(node.key()).addAttribute(field.value(),smallerNeighboursRatio);
        });
    }
};

