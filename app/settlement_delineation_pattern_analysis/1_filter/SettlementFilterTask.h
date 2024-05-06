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

template<fishnet::geometry::IPolygon P>
class SettlementFilterTask {
private:
    fishnet::Shapefile input;
    fishnet::util::AllOfPredicate<P> unaryCompositeFilter;
    fishnet::util::AllOfPredicate<P,P> binaryCompositeFilter;
    fishnet::Shapefile output;
public:
    SettlementFilterTask(fishnet::Shapefile  input, fishnet::Shapefile  output):input(std::move(input)),output(std::move(output)){}

    void run() const noexcept {
        auto inputLayer = fishnet::VectorLayer<P>::read(input);
        auto result = filter(inputLayer.getGeometries(), binaryCompositeFilter, unaryCompositeFilter);
        auto outputLayer = fishnet::VectorLayer<P>::empty(inputLayer.getSpatialReference());
        auto idField = outputLayer.addSizeField("FISHNET_ID");
        if(not idField){
            std::cerr << "Coult not create ID Field" << std::endl;
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

    std::string getTaskName() const noexcept {
        std::stringstream builder;
        builder << "Filter\n"
            << "\tInput: "<<getInput().getPath().filename().string() <<"\n"
            << "\tOutput:"<<getOutput().getPath().filename().string();
        return builder.str();
    }

    using json = nlohmann::json;

    static SettlementFilterTask<P> create(fishnet::Shapefile input, fishnet::Shapefile output) noexcept{
        return SettlementFilterTask(input, output);
    }
};

