#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/Shapefile.hpp>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/GraphFactory.hpp>
#include <fishnet/Contraction.hpp>
#include <fishnet/CompositePredicate.hpp>

#include "MemgraphAdjacency.hpp"
#include "SettlementPolygon.hpp"
#include "ContractionConfig.hpp"
#include "Task.hpp"
#include "IDReduceFunction.hpp"

template<fishnet::geometry::IPolygon P>
class ContractionTask:public Task{
private:
    std::vector<fishnet::Shapefile> inputs;
    ContractionConfig<P> config;
    fishnet::Shapefile output;
public:
    using ResultGeometryType = fishnet::geometry::MultiPolygon<P>;
    using SourceNodeType = SettlementPolygon<P>;
    using ResultNodeType = SettlementPolygon<ResultGeometryType>;
    ContractionTask(ContractionConfig<P> && config,fishnet::Shapefile output):config(std::move(config)),output(std::move(output)){
        this->writeDescLine("Contraction Task:");
    }

    ContractionTask<P> & addInput(fishnet::Shapefile && shpFile) noexcept {
        inputs.push_back(std::move(shpFile));
        return *this;
    }

    fishnet::util::forward_range_of<SettlementPolygon<P>> auto readInputs( MemgraphAdjacency<SourceNodeType> & adj, OGRSpatialReference & spatialRef) {
        std::vector<SettlementPolygon<P>> polygons;
        for(const auto & shp : inputs) {
            this->indentDescLine(shp.getPath().filename().string());
            auto layer = fishnet::VectorLayer<P>::read(shp);
            auto fileRef = adj.getDatabaseConnection().addFileReference(shp.getPath().filename().string());
            if(not fileRef){
                throw std::runtime_error("Could not read file reference for shp file:\n"+shp.getPath().string());
            }
            auto optFishnetIdField = layer.getSizeField(Task::FISHNET_ID_FIELD);
            spatialRef = layer.getSpatialReference();
            if(not optFishnetIdField) {
                throw std::runtime_error("Could not find FISHNET_ID field in shp file: \n"+shp.getPath().string());
            }
            for(const auto & feature : layer.getFeatures()) {
                auto optId = feature.getAttribute(optFishnetIdField.value());
                if(not optId){
                    throw std::runtime_error("No id exists for feature with geometry:\n"+ feature.getGeometry().toString());
                }
                polygons.emplace_back(optId.value(),fileRef.value(),std::move(feature.getGeometry()));
            }
        
        }
        adj.load(polygons);
        return polygons;
    }

    void run() override {
        if(inputs.size() < 1){
            throw std::runtime_error( "No input file provided");
        }
        auto memgraphAdjSrc = MemgraphAdjacency<SourceNodeType>::create(config.params);
        auto memgraphAdjRes = MemgraphAdjacency<ResultNodeType>::create(config.params);
        OGRSpatialReference ref;
        testExpectedOrThrowError(memgraphAdjSrc);
        testExpectedOrThrowError(memgraphAdjRes);
        auto settlements = readInputs(memgraphAdjSrc.value(),ref);
        auto outputFileRef = memgraphAdjSrc->getDatabaseConnection().addFileReference(output.getPath().filename().string());
        if(not outputFileRef)
            throw std::runtime_error( "Could not create file reference in Database for: "+output.getPath().string());
        auto sourceGraph = fishnet::graph::GraphFactory::UndirectedGraph<SourceNodeType>(std::move(memgraphAdjSrc.value()));
        auto resultGraph = fishnet::graph::GraphFactory::UndirectedGraph<ResultNodeType>(std::move(memgraphAdjRes.value()));
        auto reduceFunction = IDReduceFunction(outputFileRef.value());
        auto contractionPredicate = fishnet::util::AllOfPredicate<SourceNodeType,SourceNodeType>();
        std::ranges::for_each(config.contractBiPredicates,[&contractionPredicate](const auto & predicate){contractionPredicate.add(
            [&predicate](const SourceNodeType & lhs, const SourceNodeType & rhs){return predicate(static_cast<P>(lhs),static_cast<P>(rhs));});
        });
        fishnet::graph::contractInPlace(sourceGraph,contractionPredicate,reduceFunction,resultGraph,config.workers);
        sourceGraph.clear();
        auto outputLayer = fishnet::VectorLayer<ResultGeometryType>::empty(ref);
        auto idFieldExp = outputLayer.addSizeField(Task::FISHNET_ID_FIELD);
        if(not idFieldExp)
            throw std::runtime_error(idFieldExp.error());
        const auto & idField = idFieldExp.value();
        for(const auto & node: resultGraph.getNodes()){
            fishnet::Feature f {static_cast<ResultGeometryType>(node)};
            f.addAttribute(idField,node.key());
            outputLayer.addFeature(std::move(f));
        }
        outputLayer.overwrite(output);
    }
};