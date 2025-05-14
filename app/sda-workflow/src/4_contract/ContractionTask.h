#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/Shapefile.hpp>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/GraphFactory.hpp>
#include <fishnet/Contraction.hpp>
#include <fishnet/CompositePredicate.hpp>

#include <fishnet/CachingMemgraphAdjacency.hpp>
#include <fishnet/MemgraphAdjacency.hpp>
#include <fishnet/Task.hpp>
#include "SettlementPolygon.hpp"
#include "ContractionConfig.hpp"
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
    ContractionConfig config;
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
    ContractionTask(ContractionConfig && config,std::vector<ComponentReference> && components,fishnet::Shapefile output,size_t workflowID):Task(workflowID),components(std::move(components)),config(std::move(config)),output(std::move(output)){
        this->desc["type"]="CONTRACTION";
        this->desc["config"]=this->config.jsonDescription;
        std::vector<std::string> componentStrings;
        std::ranges::for_each(this->components,[&componentStrings](auto compRef){componentStrings.push_back(std::to_string(compRef.componentId));});
        this->desc["components"]= componentStrings;
        this->desc["output"] = this->output.getPath().filename().string();
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
    std::vector<SettlementPolygon<P>> readInputs( CachingMemgraphAdjacency<SourceNodeType> & adj, OGRSpatialReference & spatialRef) {
        std::vector<SettlementPolygon<P>> polygons;
        std::vector<std::string> inputStrings;
        std::ranges::for_each(this->inputs,[&inputStrings](auto const & file){inputStrings.push_back(file.getPath().filename().string());});
        this->desc["inputs"]=inputStrings;
        for(const auto & shp : inputs) {
            auto layer = fishnet::VectorLayer<P>::read(shp);
            if(spatialRef.IsEmpty())
                spatialRef = layer.getSpatialReference();
            if(not spatialRef.IsSame(&layer.getSpatialReference()))
                throw std::runtime_error("Spatial reference of files do not match!\nExpecting: "+std::string(spatialRef.GetName())+"\nActual: "+layer.getSpatialReference().GetName());
            if(layer.isEmpty())
                continue;
            auto fileRef = adj.getDatabaseConnection().addFileReference(shp.getPath());
            if(not fileRef){
                throw std::runtime_error("Could not read file reference for shp file:\n"+shp.getPath().string());
            }
            auto optFishnetIdField = layer.getSizeField(Task::FISHNET_ID_FIELD);
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
        MemgraphConnection memgraphConnection = MemgraphConnection::create(config.params,workflowID).value_or_throw();
        auto memgraphAdjSrc = CachingMemgraphAdjacency<SourceNodeType>(MemgraphClient(MemgraphConnection(memgraphConnection)));
        auto memgraphAdjRes = MemgraphAdjacency<ResultNodeType>(MemgraphClient(MemgraphConnection(memgraphConnection)));
        OGRSpatialReference ref; // set by readInputs function, used as spatial reference for output layer
        auto settlements = readInputs(memgraphAdjSrc,ref);
        auto outputFileRef = memgraphAdjSrc.getDatabaseConnection().addFileReference(output.getPath());
        if(not outputFileRef)
            throw std::runtime_error( "Could not create file reference for output in Database: "+output.getPath().string());
        auto sourceGraph = fishnet::graph::GraphFactory::UndirectedGraph<SourceNodeType>(std::move(memgraphAdjSrc));
        this->desc["#Nodes-before-contraction"]=fishnet::util::size(sourceGraph.getNodes());
        auto resultGraph = fishnet::graph::GraphFactory::UndirectedGraph<ResultNodeType>(std::move(memgraphAdjRes));
        /*Reduce function used to merge a connected component of nodes (SourceNodeType), solely connected via to-be-contracted edges, into a single node of the ResultNodeType*/
        auto reduceFunction = IDReduceFunction(outputFileRef.value());
        auto contractionPredicate = fishnet::util::AllOfPredicate<SourceNodeType,SourceNodeType>();
        /* Load all contraction predicates into a single composite contraction predicate */
        std::ranges::for_each(config.initContractionPredicates<P>(distanceFunctionForSpatialReference(ref)),[&contractionPredicate](auto && p){contractionPredicate.add(
            [predicate=std::move(p)](const SourceNodeType & lhs, const SourceNodeType & rhs){return predicate(static_cast<P>(lhs),static_cast<P>(rhs));});
        });
        /* Contract the graph according to the composite contraction predicate. Old adjacencies are removed from the database as well to allow the reuse of ids, while remaining consistency. */
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
        this->desc["#Nodes-after-contraction"]=outputLayer.size();
        outputLayer.overwrite(output);
    }
};