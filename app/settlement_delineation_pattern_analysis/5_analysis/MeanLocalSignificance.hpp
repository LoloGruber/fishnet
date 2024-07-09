#pragma once
#include <fishnet/Graph.hpp>
#include <fishnet/Feature.hpp>
#include <fishnet/ThreadPool.hpp>
#include <fishnet/PolygonDistance.hpp>
#include "CentralityMeasureType.hpp"

#include <fishnet/StopWatch.h>

struct MeanLocalSignificance{
    constexpr static size_t NODES_PER_THREAD = 100;
    static CentralityMeasureType type() noexcept {
        return CentralityMeasureType::MeanLocalSignificance;
    }

    template<fishnet::graph::Graph GraphType, typename GeometryType>
    void operator()(const GraphType & source, fishnet::VectorLayer<GeometryType> & layer, std::unordered_map<size_t,fishnet::Feature<GeometryType>> & result) const {
        auto field = layer.addDoubleField("MeanLocSig");
        if(not field){
            throw std::runtime_error("Could not create field \"MeanLocSig\"");
        }
        auto computeMeanLocalSig = [&source,&field,&result](const auto & node){
            double accLocalSig = 0;
            int count = 0;
            for(const auto & neighbour : source.getNeighbours(node)){
                auto distance = fishnet::geometry::shapeDistance(node,neighbour);
                accLocalSig+= (node.area() * neighbour.area()) / pow(distance,2);
                count++;
            }
            double meanLocalSig = count==0?0.0:accLocalSig / count;
            result.at(node.key()).addAttribute(field.value(),meanLocalSig);
        };
        const auto & nodes = source.getNodes();
        auto threadPoolSize = std::min(static_cast<size_t>(std::thread::hardware_concurrency()/2),static_cast<size_t>(fishnet::util::size(nodes)/NODES_PER_THREAD + 1));
        if(threadPoolSize > 1){
            fishnet::util::ThreadPool pool {threadPoolSize};
            for(const auto & node: nodes){
                pool.submit([&computeMeanLocalSig,&node]{computeMeanLocalSig(node);});
            }
            pool.join();
        }else{
            std::ranges::for_each(nodes,computeMeanLocalSig);
        }
    }
};
