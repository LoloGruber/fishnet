#pragma once
#include <fishnet/VectorLayer.hpp>
#include <fishnet/Feature.hpp>
#include <fishnet/Shapefile.hpp>
#include <fishnet/CompositePredicate.hpp>
#include <fishnet/GeometryObject.hpp>
#include <fishnet/PolygonFilter.hpp>

#include <sstream>

#include <fishnet/Task.hpp>
#include "FilterConfig.hpp"

/**
 * @brief Implementation of the filter step of the workflow.
 * Use composite unary and binary filters to test the settlements stored in the input shapefile.
 * Stores the settlements, passing the filter, in the output shapefile with a unique FishnetID
 * @tparam P polygon type 
 */
template<fishnet::geometry::IPolygon P>
class SettlementFilterTask: public Task {
private:
    FilterConfig<P> config;
    fishnet::Shapefile input;
    fishnet::util::AllOfPredicate<P> unaryCompositeFilter;
    fishnet::util::AllOfPredicate<P,P> binaryCompositeFilter;
    fishnet::Shapefile output;
public:
    SettlementFilterTask(FilterConfig<P> && config,fishnet::Shapefile  input, fishnet::Shapefile  output):Task(),config(std::move(config)),input(std::move(input)),output(std::move(output)){
        this->desc["type"]="FILTER";
        this->desc["input"]=this->input.getPath().filename().string();
        this->desc["config"]=this->config.jsonDescription;
        this->desc["output"]=this->output.getPath().filename().string();
    }

    void run() override{
        std::ranges::for_each(config.unaryPredicates,[this](const auto & filter){unaryCompositeFilter.add(filter);});
        std::ranges::for_each(config.binaryPredicates,[this](const auto & binaryFilter){binaryCompositeFilter.add(binaryFilter);});
        auto inputLayer = fishnet::VectorLayer<P>::read(input);
        auto result = filter(inputLayer.getGeometries(), binaryCompositeFilter, unaryCompositeFilter);
        auto outputLayer = fishnet::VectorLayer<P>::empty(inputLayer.getSpatialReference());
        auto idField = outputLayer.addSizeField(Task::FISHNET_ID_FIELD);
        if(not idField){
            throw std::runtime_error("Could not create ID Field");
        }
        auto polygonHasher = std::hash<P>();
        for(auto && geometry: result) {
            fishnet::Feature<P> current {std::move(geometry)};
            current.addAttribute(*idField,normalizeToShpFileIntField(polygonHasher(current.getGeometry())));
            outputLayer.addFeature(std::move(current));
        }
        outputLayer.overwrite(output);
        this->desc["polygon count"]=outputLayer.size();
    }

    SettlementFilterTask & setInput(fishnet::Shapefile && inputFile)noexcept{
        this->input = std::move(inputFile);
        return *this;
    }

    SettlementFilterTask & setInput(const fishnet::Shapefile&  inputFile)noexcept{
        this->input = inputFile;
        return *this;
    }

    SettlementFilterTask & addPredicate(fishnet::util::Predicate<P> auto && predicate) noexcept {
        this->unaryCompositeFilter.add(predicate);
        return *this;
    }

    SettlementFilterTask & addBiPredicate(fishnet::util::BiPredicate<P,P> auto && predicate) noexcept {
        this->binaryCompositeFilter.add(predicate);
        return *this;
    }

    SettlementFilterTask & setConfig(FilterConfig<P> && filterConfig) noexcept {
        this->config = std::move(filterConfig);
        return *this;
    }

    SettlementFilterTask & setConfig(const FilterConfig<P> & filterConfig ) noexcept {
        this->config = filterConfig;
        return *this;
    }

    SettlementFilterTask & setOutput(fishnet::Shapefile && outputFile) noexcept {
        this->output = std::move(outputFile);
        return *this;
    }

    SettlementFilterTask & setOutput(const fishnet::Shapefile& outputFile) noexcept {
        this->output = outputFile;
        return *this;
    }

    const fishnet::Shapefile & getInput() const noexcept {
        return this->input;
    }

    const fishnet::Shapefile & getOutput() const noexcept {
        return this->output;
    }

    const FilterConfig<P> & getConfig() const noexcept {
        return this->config;
    }
};

