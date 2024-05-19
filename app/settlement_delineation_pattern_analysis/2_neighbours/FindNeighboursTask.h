#pragma once
#include <vector>
#include <fishnet/Shapefile.hpp>
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/PolygonNeighbours.hpp>
#include <fishnet/GraphFactory.hpp>
#include <fishnet/CompositePredicate.hpp>
#include <fishnet/Rectangle.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include "MemgraphAdjacency.hpp"
#include "SettlementPolygon.hpp"
#include "Task.hpp"
#include "FindNeighboursConfig.hpp"


template<fishnet::geometry::IPolygon P>
class FindNeighboursTask : public Task{
private:
    std::vector<fishnet::Shapefile> inputs;
    FindNeighboursConfig<P> config;
public:
    FindNeighboursTask(FindNeighboursConfig<P> && config):config(std::move(config)){
        this->writeDescLine("Find Neighbours Task:");
    }

    FindNeighboursTask<P> & addShapefile(fishnet::Shapefile && shpFile) noexcept {
        inputs.push_back(std::move(shpFile));
        return *this;
    }

    template<fishnet::util::BiPredicate<P> NeighbourBiPredicate>
    FindNeighboursTask<P> & addNeighbouringPredicate(NeighbourBiPredicate && predicate) noexcept {
        config.neighbouringPredicates.push_back(std::forward<NeighbourBiPredicate>(predicate));
        return *this;
    }

    FindNeighboursTask<P> & setMaxEdgeDistance(double distanceInMeters) noexcept {
        config.maxEdgeDistance = distanceInMeters;
        return *this;
    }

    FindNeighboursTask<P> & setMemgraphParams(mg::Client::Params && params) noexcept {
        config.memgraphParams = std::move(params);
        return *this;
    }

    FindNeighboursTask<P> & setConfig(FindNeighboursConfig<P> && neighboursConfig) noexcept  {
        this->config = std::move(neighboursConfig);
        return *this;
    }

    FindNeighboursTask<P> & setConfig(const FindNeighboursConfig<P> & neighboursConfig ) noexcept {
        this->config = neighboursConfig;
        return *this;
    }

    FindNeighboursTask<P> & setMemgraphParams(std::string const & hostname, uint16_t port) noexcept {
        mg::Client::Params params;
        params.host = hostname;
        params.port = port;
        return setMemgraphParams(std::move(params));
    }

    void run() noexcept override{
        fishnet::util::AllOfPredicate<P,P> neighbouringPredicate;
        std::ranges::for_each(config.neighbouringPredicates,[&neighbouringPredicate](const auto & predicate){neighbouringPredicate.add(predicate);});
        auto expAdj = MemgraphAdjacency<SettlementPolygon<P>>::create(config.params);
        testExpectedOrThrowError(expAdj);
        auto graph = fishnet::graph::GraphFactory::UndirectedGraph<SettlementPolygon<P>>(std::move(expAdj.value()));
        std::vector<SettlementPolygon<P>> polygons;
        this->writeDescLine("-Config:");
        this->indentDescLine(config.jsonDescription.dump());
        this->writeDescLine("-Inputs: ");
        if(inputs.size() < 1)
            std::cerr << "No input file provided" << std::endl;
        for(const auto & shp : inputs) {
            this->indentDescLine(shp.getPath().filename().string());
            auto layer = fishnet::VectorLayer<P>::read(shp);
            auto fileRef = graph.getAdjacencyContainer().getDatabaseConnection().addFileReference(shp.getPath().filename().string());
            if(not fileRef){
                std::cerr << "Could not create file reference for shp file:\n"+shp.getPath().string() << std::endl;
                return;
            }
            auto optFishnetIdField = layer.getSizeField(Task::FISHNET_ID_FIELD);
            if(not optFishnetIdField) {
                std::cerr << "Could not find FISHNET_ID field in shp file: \n"+shp.getPath().string() << std::endl;
                return;
            }
            for(const auto & feature : layer.getFeatures()) {
                auto optId = feature.getAttribute(optFishnetIdField.value());
                if(not optId){
                    std::cerr << "No id exists for feature with geometry:\n"<< feature.getGeometry() << std::endl;
                }
                polygons.emplace_back(optId.value(),fileRef.value(),std::move(feature.getGeometry()));
            }
        }

        double maxEdgeDistanceVar = config.maxEdgeDistance;
        auto boundingBoxPolygonWrapper = [maxEdgeDistanceVar](const SettlementPolygon<P> & settPolygon ){
            auto aaBB = fishnet::geometry::Rectangle<fishnet::math::DEFAULT_NUMERIC>(settPolygon.aaBB().getPoints());
            double distanceMetersTopLeftBotLeft = fishnet::WGS84Ellipsoid::distance(aaBB.left(),aaBB.top(),aaBB.left(),aaBB.bottom());
            double scale = maxEdgeDistanceVar / distanceMetersTopLeftBotLeft;
            return fishnet::geometry::BoundingBoxPolygon(settPolygon,aaBB.scale(scale));
        };
        auto result = fishnet::geometry::findNeighbouringPolygons(polygons,neighbouringPredicate,boundingBoxPolygonWrapper);
        graph.addNodes(polygons);
        graph.addEdges(result);
    }
};