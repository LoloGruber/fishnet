#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/Shapefile.hpp>
#include <fishnet/VectorLayer.hpp>

#include "ContractionConfig.hpp"
#include "Task.hpp"

template<fishnet::geometry::IPolygon P>
class ContractionTask:public Task{
private:
    std::vector<fishnet::Shapefile> inputs;
    ContractionConfig<P> config;
public:
    ContractionTask(){
        this->writeDescLine("Contraction Task:");
    }

    void run() noexcept override {
        
    }
};