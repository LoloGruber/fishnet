#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/Shapefile.hpp>
#include <fishnet/GraphFactory.hpp>
#include "Task.hpp"
#include "AnalysisConfig.cpp"
#include "CentralityMeasureJsonReader.hpp"
#include "MemgraphAdjacency.hpp"
#include "SettlementPolygon.hpp"
#include "EdgeVisualizer.hpp"

template<fishnet::geometry::Shape ShapeType>
class AnalysisTask: public Task{
private:
    AnalysisConfig config;
    fishnet::Shapefile inputFile;
    fishnet::Shapefile outputFile;

public:
    using NodeType = SettlementPolygon<ShapeType>;

    AnalysisTask(AnalysisConfig && config, fishnet::Shapefile inputFile, fishnet::Shapefile outputFile):config(std::move(config)),inputFile(std::move(inputFile)),outputFile(std::move(outputFile)){
        this->writeDescLine("Analysis Task:")
            .writeDescLine("-Config:")
            .indentDescLine(this->config.jsonDescription.dump())
            .writeDescLine("-Input:")
            .indentDescLine(this->inputFile.getPath().filename().string())
            .writeDescLine("-Output:")
            .indentDescLine(this->outputFile.getPath().filename().string());
    }

    std::vector<NodeType> readInput(const MemgraphAdjacency<NodeType> & adj ,OGRSpatialReference & ref) const  {
        std::vector<NodeType> settlements;
        auto layer = fishnet::VectorLayer<ShapeType>::read(inputFile);
        auto fileRef = adj.getDatabaseConnection().addFileReference(inputFile.getPath().filename().string());
        if(not fileRef){
            throw std::runtime_error("Could not read file reference for shp file:\n"+inputFile.getPath().string());
        }
        auto optFishnetIdField = layer.getSizeField(Task::FISHNET_ID_FIELD);
        if(not optFishnetIdField) {
            throw std::runtime_error("Could not find FISHNET_ID field in shp file: \n"+inputFile.getPath().string());
        }
        for(auto & feature : layer.getFeatures()) {
            auto optId = feature.getAttribute(optFishnetIdField.value());
            if(not optId){
               throw std::runtime_error("No id exists for feature with geometry:\n"+ feature.getGeometry().toString());
            }
            auto id = optId.value();
            settlements.emplace_back(id,fileRef.value(),std::move(feature.getGeometry()));

        }
        ref = layer.getSpatialReference();
        return settlements;
    }

    void run() override {
        auto memgraphAdjExp = MemgraphAdjacency<NodeType>::create(config.params);
        testExpectedOrThrowError(memgraphAdjExp);
        OGRSpatialReference outputRef;
        auto settlements = readInput(memgraphAdjExp.value(),outputRef);
        memgraphAdjExp->load(settlements);
        auto graph = fishnet::graph::GraphFactory::UndirectedGraph<NodeType>(std::move(memgraphAdjExp.value()));
        std::unordered_map<size_t,fishnet::Feature<ShapeType>> centralityMeasureResults;
        std::ranges::for_each(settlements,[&centralityMeasureResults](auto && settlement){
            centralityMeasureResults.try_emplace(settlement.key(),fishnet::Feature<ShapeType>(std::move(static_cast<ShapeType>(settlement))));
        });
        fishnet::VectorLayer<ShapeType> outputLayer = fishnet::VectorLayer<ShapeType>::empty(outputRef);
        for(auto centralityMeasure : config.loadCentralityMeasures<decltype(graph),ShapeType>()) {
            centralityMeasure(graph,outputLayer,centralityMeasureResults);
        }
        auto fishnetIDField = outputLayer.addSizeField(Task::FISHNET_ID_FIELD);
        if(not fishnetIDField)
            throw std::runtime_error(fishnetIDField.error());
        for(auto && [fishnetID, feature]: centralityMeasureResults){
            feature.addAttribute(fishnetIDField.value(),fishnetID);
            outputLayer.addFeature(std::move(feature));
        }
        outputLayer.overwrite(outputFile);
        if(config.visualizeEdges) {
            auto edgeFile = outputFile.appendToFilename("_edges");
            auto edgeLayer = fishnet::VectorLayer<fishnet::geometry::SimplePolygon<double>>::empty(outputRef);
            for(const auto & edge : graph.getEdges()){
                auto edgePolygon = visualizeEdge(edge.getFrom(),edge.getTo());
                if (not edgePolygon){
                    std::cerr << "Could not create edge\nFrom:"<<edge.getFrom() <<"\nTo:" << edge.getTo() << std::endl;
                }
                edgeLayer.addGeometry(std::move(edgePolygon.value()));
            }
            edgeLayer.overwrite(edgeFile);
        }
    }
};