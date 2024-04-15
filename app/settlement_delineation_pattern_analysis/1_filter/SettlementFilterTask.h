//
// Created by lolo on 09.04.24.
//
#pragma once

#include "VectorLayer.hpp"
#include "Shapefile.hpp"
#include <fishnet/CompositePredicate.hpp>

#include <fishnet/GeometryObject.hpp>
#include <fishnet/PolygonFilter.hpp>

#include <nlohmann/json.hpp> // Copyright (c) 2013-2022 Niels Lohmann

template<fishnet::geometry::IPolygon P>
class SettlementFilterTask {
private:
    fishnet::Shapefile input;
    util::AllOfPredicate<P> unaryCompositeFilter;
    util::AllOfPredicate<P,P> binaryCompositeFilter;
    fishnet::Shapefile output;

public:
    SettlementFilterTask(fishnet::Shapefile  input, fishnet::Shapefile  output):input(std::move(input)),output(std::move(output)){}

    void run() const noexcept {
        auto inputLayer = fishnet::VectorLayer<P>::read(input);
        auto result = filter(inputLayer.getGeometries(), binaryCompositeFilter, unaryCompositeFilter);
        auto outputLayer = fishnet::VectorLayer<P>::empty(inputLayer.getSpatialReference());
        outputLayer.addAllGeometry(result);
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

    SettlementFilterTask & addPredicate(util::Predicate<P> auto && predicate) noexcept {
        this->unaryCompositeFilter.add(predicate);
        return *this;
    }

    SettlementFilterTask & addBiPredicate(util::BiPredicate<P,P> auto && predicate) noexcept {
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
    using json = nlohmann::json;

    static std::expected<SettlementFilterTask<P>,std::string> fromJson(const json & j) noexcept {
        std::string inputFilename;
        j.at("input").get_to(inputFilename);
        std::string outputFilename;
        j.at("output").get_to(outputFilename);
        fishnet::Shapefile input{inputFilename};
        if(not input.exists())
            return std::unexpected("Input file: \"" + inputFilename + "\" does not exist.");
        fishnet::Shapefile output{outputFilename};
        if(outputFilename.empty())
            output = input.appendToFilename("_filtered");
        //TODO
    }
};

