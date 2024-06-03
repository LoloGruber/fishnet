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

/**
 * @brief Implementation of the neighbours task. Finds neighbouring settlements according to the neighbouring predicates specified in the configuration file
 * 
 * @tparam P polygon type
 */
template<fishnet::geometry::IPolygon P>
class FindNeighboursTask : public Task{
private:
    FindNeighboursConfig<P> config;
    fishnet::Shapefile primaryInput;
    std::vector<fishnet::Shapefile> additionalInput;


    using number = typename P::numeric_type;
    struct PrimaryInputAABB{
        number left = std::numeric_limits<number>::max();
        number right = std::numeric_limits<number>::min();
        number top = std::numeric_limits<number>::min();
        number bottom = std::numeric_limits<number>::max();

        void update(const P & polygon) {
            auto aaBB = fishnet::geometry::Rectangle<number>(polygon);
            if(aaBB.left() < left)
                left = aaBB.left();
            if(aaBB.right() > right)
                right = aaBB.right();
            if(aaBB.top() > top)
                top = aaBB.top();
            if(aaBB.bottom() < bottom)
                bottom = aaBB.bottom();
        }

        fishnet::geometry::Rectangle<number> asShape() const noexcept {
            return fishnet::geometry::Rectangle<number>({{left,top},{right,top},{right,bottom},{left,bottom}});
        }


    };

public:
    FindNeighboursTask(FindNeighboursConfig<P> && config,fishnet::Shapefile primaryInput):config(std::move(config)),primaryInput(std::move(primaryInput)){
        this->writeDescLine("Find Neighbours Task:");
        this->writeDescLine("-Config:");
        this->indentDescLine(config.jsonDescription.dump());
        this->writeDescLine("-Inputs: ");
        this->indentDescLine(primaryInput.getPath().filename().string());
    }

    FindNeighboursTask<P> & addShapefile(fishnet::Shapefile && shpFile) noexcept {
        additionalInput.push_back(std::move(shpFile));
        return *this;
    }

    template<fishnet::util::BiPredicate<P> NeighbourBiPredicate>
    FindNeighboursTask<P> & addNeighbouringPredicate(NeighbourBiPredicate && predicate) noexcept {
        config.neighbouringPredicates.push_back(std::forward<NeighbourBiPredicate>(predicate));
        return *this;
    }

    std::vector<SettlementPolygon<P>> readInput(auto const & graph)  {
        std::vector<SettlementPolygon<P>> polygons;
        auto layer = fishnet::VectorLayer<P>::read(primaryInput); // load polygons from primary shapefile
        PrimaryInputAABB inputBoundingBox;
        auto primaryFileRef = graph.getAdjacencyContainer().getDatabaseConnection().addFileReference(primaryInput.getPath()); // load file reference from database
        if(not primaryFileRef){
            throw std::runtime_error("Could not create file reference for shp file:\n"+primaryInput.getPath().string());
        }
        auto primaryOptFishnetIdField = layer.getSizeField(Task::FISHNET_ID_FIELD);
        if(not primaryOptFishnetIdField) {
            throw std::runtime_error("Could not find FISHNET_ID field in shp file: \n"+primaryInput.getPath().string());
        }
        for(const auto & feature : layer.getFeatures()) {
            auto optId = feature.getAttribute(primaryOptFishnetIdField.value()); // read FISHNET_ID of feature
            if(not optId){
                throw std::runtime_error("No id exists for feature with geometry:\n"+feature.getGeometry().toString());
            }
            inputBoundingBox.update(feature.getGeometry());
            polygons.emplace_back(optId.value(),primaryFileRef.value(),std::move(feature.getGeometry())); // create settlement wrapper containing its unique id and geometry
        }
        DistanceBiPredicate distanceToPrimaryInput {config.maxEdgeDistance};
        fishnet::geometry::Rectangle<number> primaryInputAABB = inputBoundingBox.asShape();
        for(const auto & shp : additionalInput) {
            this->indentDescLine(shp.getPath().filename().string());
            auto layer = fishnet::VectorLayer<P>::read(shp); // load polygons from shapefile
            auto fileRef = graph.getAdjacencyContainer().getDatabaseConnection().addFileReference(shp.getPath()); // load file reference from database
            if(not fileRef){
                throw std::runtime_error("Could not create file reference for shp file:\n"+shp.getPath().string());
            }
            auto optFishnetIdField = layer.getSizeField(Task::FISHNET_ID_FIELD);
            if(not optFishnetIdField) {
                throw std::runtime_error("Could not find FISHNET_ID field in shp file: \n"+shp.getPath().string());
            }
            for(const auto & feature : layer.getFeatures()) {
                auto optId = feature.getAttribute(optFishnetIdField.value()); // read FISHNET_ID of feature
                if(not optId){
                    throw std::runtime_error("No id exists for feature with geometry:\n"+feature.getGeometry().toString());
                }
                if(distanceToPrimaryInput(primaryInputAABB,feature.getGeometry())) // consider only polygons in range of the primary input
                    polygons.emplace_back(optId.value(),fileRef.value(),std::move(feature.getGeometry())); // create settlement wrapper containing its unique id and geometry
            }
        }
        return polygons;
    }

    void run() override{
        fishnet::util::AllOfPredicate<P,P> neighbouringPredicate;
        /* add all neighbouring predicates to composite predicate */
        std::ranges::for_each(config.neighbouringPredicates,[&neighbouringPredicate](const auto & predicate){neighbouringPredicate.add(predicate);});
        auto expAdj = MemgraphAdjacency<SettlementPolygon<P>>::create(config.params);
        testExpectedOrThrowError(expAdj);
        auto graph = fishnet::graph::GraphFactory::UndirectedGraph<SettlementPolygon<P>>(std::move(expAdj.value()));       
        std::vector<SettlementPolygon<P>> polygons = readInput(graph);
        double maxEdgeDistanceVar = config.maxEdgeDistance;
        auto boundingBoxPolygonWrapper = [maxEdgeDistanceVar](const SettlementPolygon<P> & settPolygon ){
            /* Create scaled aaBB containing at least all points reachable from the polygon within the maximum edge distance*/
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