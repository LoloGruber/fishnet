#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/Shapefile.hpp>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/GraphFactory.hpp>
#include <fishnet/Contraction.hpp>
#include <fishnet/CompositePredicate.hpp>

#include "CachingMemgraphAdjacency.hpp"
#include "MemgraphAdjacency.hpp"
#include "SettlementPolygon.hpp"
#include "ContractionConfig.hpp"
#include "Task.hpp"
#include "IDReduceFunction.hpp"

/**
 * @brief Implementation of the contraction task. 
 * The graph of all settlements stored in the input files (and part of specified connected components) are first loaded from the database.
 * Edges between settlements fulfilling the composite contraction predicate are contracted, and the adjacent settlements merged into a single entity (e.g. a Multi-Polygon containing all settlements)
 * @tparam P polygon type of the settlements
 */
template<fishnet::geometry::IPolygon P>
class ContractionTask:public Task{
private:
    std::vector<fishnet::Shapefile> inputs;
    std::vector<ComponentReference> components;
    ContractionConfig<P> config;
    fishnet::Shapefile output;
public:
    /**
     * @brief geometry type of the result (defines reduce function output type)
     */
    using ResultGeometryType = fishnet::geometry::MultiPolygon<P>;
    /**
     * @brief settlement type before the contraction
     */
    using SourceNodeType = SettlementPolygon<P>; 
    /**
     * @brief settlement type after the contraction
     */
    using ResultNodeType = SettlementPolygon<ResultGeometryType>;
    ContractionTask(ContractionConfig<P> && config,std::vector<ComponentReference> && components,fishnet::Shapefile output):components(std::move(components)),config(std::move(config)),output(std::move(output)){
        this->writeDescLine("Contraction Task:")
        .writeDescLine("-Config:")
        .indentDescLine(this->config.jsonDescription.dump())
        .writeDescLine("-Components:");
        std::stringstream componentsString;
        for(const auto & component: this->components){
            componentsString << component.componentId <<",";
        }
        this->indentDescLine(componentsString.str());
    }

    ContractionTask<P> & addInput(fishnet::Shapefile && shpFile) noexcept {
        inputs.push_back(std::move(shpFile));
        return *this;
    }

    /**
     * @brief Helper function to read the settlements from the shape files and load their relationships from the memgraph database.
     * Additionally sets the out parameter spatialRef, to the spatial reference system used in the inputs 
     * @param adj IN_OUT memgraph adjacency instance, loads the settlement relationships
     * @param spatialRef IN_OUT spatial reference used for the ouput shapefile, set according to input spatial reference
     * @throws runtime_error when the file reference for the inputs could not be loaded or the id of a settlement could not be read
     * @return fishnet::util::forward_range_of<SettlementPolygon<P>> list of settlements
     */
    fishnet::util::forward_range_of<SettlementPolygon<P>> auto readInputs( CachingMemgraphAdjacency<SourceNodeType> & adj, OGRSpatialReference & spatialRef) {
        std::vector<SettlementPolygon<P>> polygons;
        this->writeDescLine("-Inputs:");
        for(const auto & shp : inputs) {
            this->indentDescLine(shp.getPath().filename().string());
            auto layer = fishnet::VectorLayer<P>::read(shp);
            auto fileRef = adj.getDatabaseConnection().addFileReference(shp.getPath());
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
        if(not adj.loadNodes(polygons,components)){
            throw std::runtime_error("Could not load nodes from components");
        }
        return polygons;
    }

    void run() override {
        if(inputs.empty()){
            throw std::runtime_error( "No input file provided");
        }
        auto memgraphConnection = MemgraphConnection::create(config.params);
        testExpectedOrThrowError(memgraphConnection);
        auto memgraphAdjSrc = CachingMemgraphAdjacency<SourceNodeType>(memgraphConnection.transform([](auto && con){return MemgraphClient(std::move(con));}).value());
        auto memgraphAdjRes = MemgraphAdjacency<ResultNodeType>::create(config.params);
        OGRSpatialReference ref; // set by readInputs function, used as spatial reference for output layer
        testExpectedOrThrowError(memgraphAdjRes);
        auto settlements = readInputs(memgraphAdjSrc,ref);
        this->writeDescLine("-Output:");
        this->indentDescLine(output.getPath().filename().string());
        auto outputFileRef = memgraphAdjSrc.getDatabaseConnection().addFileReference(output.getPath());
        if(not outputFileRef)
            throw std::runtime_error( "Could not create file reference for output in Database: "+output.getPath().string());
        auto sourceGraph = fishnet::graph::GraphFactory::UndirectedGraph<SourceNodeType>(std::move(memgraphAdjSrc));
        auto resultGraph = fishnet::graph::GraphFactory::UndirectedGraph<ResultNodeType>(std::move(memgraphAdjRes.value()));
        /*Reduce function used to merge a connected component of nodes (SourceNodeType), solely connected via to-be-contracted edges, into a single node of the ResultNodeType*/
        auto reduceFunction = IDReduceFunction(outputFileRef.value());
        auto contractionPredicate = fishnet::util::AllOfPredicate<SourceNodeType,SourceNodeType>();
        /* Load all contraction predicates into a single composite contraction predicate */
        std::ranges::for_each(config.contractBiPredicates,[&contractionPredicate](const auto & predicate){contractionPredicate.add(
            [&predicate](const SourceNodeType & lhs, const SourceNodeType & rhs){return predicate(static_cast<P>(lhs),static_cast<P>(rhs));});
        });
        /* Contract the graph according to the composite contraction predicate. Done in place, to remove old adjacencies from the database as well to allow the reuse of ids, while remaining consistency*/
        fishnet::graph::contractInPlace(sourceGraph,contractionPredicate,reduceFunction,resultGraph,config.workers);
        auto outputLayer = fishnet::VectorLayer<ResultGeometryType>::empty(ref);
        auto idFieldExp = outputLayer.addSizeField(Task::FISHNET_ID_FIELD); // add id field to output as well
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