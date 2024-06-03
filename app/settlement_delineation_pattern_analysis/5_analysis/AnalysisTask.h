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

/**
 * @brief Implementation of the analysis task.
 * The settlement shapes with their id are loaded from the input file (as Polygons or MultiPolygons, depending on the contraction step).
 * Their relationships are loaded from the memgraph database.
 * The selected centrality measures are applied to the settlement graph and the computed values stored as fields in the output file.
 * If desired the edges between the settlements get visualized in separate file.
 * @tparam ShapeType 
 */
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
    /**
     * @brief Helper function to read the settlements with id from the shape files.
     * Additionally sets the out parameter spatialRef, to the spatial reference system used in the inputs 
     * @param adj memgraph adjacency instance to load the file reference for the input file
     * @param ref IN_OUT spatial reference used for the ouput shapefile, set according to input spatial reference
     * @throws runtime_error when the file reference for the inputs could not be loaded or the id of a settlement could not be read
     * @return fishnet::util::forward_range_of<SettlementPolygon<P>> list of settlements
     */
    std::vector<NodeType> readInput(const MemgraphAdjacency<NodeType> & adj ,OGRSpatialReference & ref) const  {
        std::vector<NodeType> settlements;
        auto layer = fishnet::VectorLayer<ShapeType>::read(inputFile);
        auto fileRef = adj.getDatabaseConnection().addFileReference(inputFile.getPath());
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
        OGRSpatialReference outputRef; // used for the ouput shapefile
        auto settlements = readInput(memgraphAdjExp.value(),outputRef);
        memgraphAdjExp->loadNodes(settlements); //load settlement relationships
        auto graph = fishnet::graph::GraphFactory::UndirectedGraph<NodeType>(std::move(memgraphAdjExp.value()));
        std::unordered_map<size_t,fishnet::Feature<ShapeType>> centralityMeasureResults; // result map, which stores: Fishnet_ID -> <Feature of the settlement stored in output>
        std::ranges::for_each(settlements,[&centralityMeasureResults](auto && settlement){
            centralityMeasureResults.try_emplace(settlement.key(),fishnet::Feature<ShapeType>(std::move(static_cast<ShapeType>(settlement))));
        });
        fishnet::VectorLayer<ShapeType> outputLayer = fishnet::VectorLayer<ShapeType>::empty(outputRef);
        for(auto centralityMeasure : config.loadCentralityMeasures<decltype(graph),ShapeType>()) {
            /*Execute each centrality measure using the settlement graph. 
            The measure is expected to create the field for the centrality measure on the layer.
            The measure is expected to store a centrality measure value for each feature*/
            centralityMeasure(graph,outputLayer,centralityMeasureResults); 
        }
        auto fishnetIDField = outputLayer.addSizeField(Task::FISHNET_ID_FIELD);
        if(not fishnetIDField)
            throw std::runtime_error(fishnetIDField.error());
        for(auto && [fishnetID, feature]: centralityMeasureResults){
            feature.addAttribute(fishnetIDField.value(),fishnetID); // add fishnet id to output layer
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