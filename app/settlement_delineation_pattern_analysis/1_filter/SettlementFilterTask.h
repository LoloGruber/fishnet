//
// Created by lolo on 09.04.24.
//
#pragma once

#include <fishnet/VectorLayer.hpp>
#include <fishnet/Feature.hpp>
#include <fishnet/Shapefile.hpp>
#include <fishnet/CompositePredicate.hpp>

#include <fishnet/GeometryObject.hpp>
#include <fishnet/PolygonFilter.hpp>

#include <nlohmann/json.hpp> // Copyright (c) 2013-2022 Niels Lohmann
#include <sstream>
#include "Task.hpp"
#include "FilterConfig.hpp"

template<fishnet::geometry::IPolygon P>
class SettlementFilterTask: public Task {
private:
    fishnet::Shapefile input;
    fishnet::util::AllOfPredicate<P> unaryCompositeFilter;
    fishnet::util::AllOfPredicate<P,P> binaryCompositeFilter;
    FilterConfig<P> config;
    fishnet::Shapefile output;
public:
    SettlementFilterTask(fishnet::Shapefile  input, fishnet::Shapefile  output):Task(),input(std::move(input)),output(std::move(output)){
        Task::operator<<("FilterTask\n")
        << "-Input:\t\t"<<getInput().getPath().filename().string() <<"\n"
        << "-Output:\t"<<getOutput().getPath().filename().string() << "\n";
    }

    void run() noexcept override{
        std::ranges::for_each(config.unaryPredicates,[this](const auto & filter){unaryCompositeFilter.add(filter);});
        std::ranges::for_each(config.binaryPredicates,[this](const auto & binaryFilter){binaryCompositeFilter.add(binaryFilter);});
        this->writeDescLine("-Config:\t"+config.jsonDescription.dump());
        auto inputLayer = fishnet::VectorLayer<P>::read(input);
        auto result = filter(inputLayer.getGeometries(), binaryCompositeFilter, unaryCompositeFilter);
        auto outputLayer = fishnet::VectorLayer<P>::empty(inputLayer.getSpatialReference());
        auto idField = outputLayer.addSizeField(Task::FISHNET_ID_FIELD);
        if(not idField){
            std::cerr << "Could not create ID Field" << std::endl;
        }
        auto polygonHasher = std::hash<P>();
        for(auto && geometry: result) {
            fishnet::Feature<P> current {std::move(geometry)};
            current.addAttribute(*idField,polygonHasher(current.getGeometry()));
            outputLayer.addFeature(std::move(current));
        }
        outputLayer.overwrite(output);
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

    static SettlementFilterTask<P> create(fishnet::Shapefile input, fishnet::Shapefile output) noexcept{
        return SettlementFilterTask(input, output);
    }
};

