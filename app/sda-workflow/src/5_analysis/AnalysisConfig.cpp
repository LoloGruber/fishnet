#pragma once
#include <unordered_map>
#include <fishnet/Graph.hpp>
#include <fishnet/TaskConfig.hpp>
#include "CentralityMeasureType.hpp"
#include "CentralityMeasureJsonReader.hpp"
#include "magic_enum.hpp"

/**
 * @brief Analysis configuration parser
 * Additionally later loads the centrality measures according to graph and geometry type from the centrality measures types and the json description 
 */
struct AnalysisConfig: public MemgraphTaskConfig {
    constexpr static const char * EDGES_FLAG_KEY = "visualize-edges";
    constexpr static const char * CENTRALITY_MEASURES_KEY = "centrality-measures";
    bool visualizeEdges;
    std::unordered_map<CentralityMeasureType,json> centralityMeasuresDescriptions; // map CentralityMeasureType -> Json Configuration for that type

    AnalysisConfig(const json & config):MemgraphTaskConfig(config){
        this->jsonDescription.at(EDGES_FLAG_KEY).get_to(this->visualizeEdges);
        for(const auto & centralityMeasureJson : this->jsonDescription.at(CENTRALITY_MEASURES_KEY)){
            std::string centralityMeasureName;
            centralityMeasureJson.at("name").get_to(centralityMeasureName);
            auto centralityMeasureType = magic_enum::enum_cast<CentralityMeasureType>(centralityMeasureName);
            if (not centralityMeasureType){
                throw std::runtime_error("Could not parse json to Centrality Measure:\n"+centralityMeasureJson.dump()+"\nCentrality measure \""+centralityMeasureName+"\" is not supported");
            }
            this->centralityMeasuresDescriptions.try_emplace(centralityMeasureType.value(),centralityMeasureJson);
        }
    }

    /**
     * @brief Loads the centrality measures according to graph and geometry type.
     * Skips centrality measures that could not be loaded for the graph / geometry types.
     * @tparam GraphType graph type
     * @tparam GeometryType geometry type
     * @return std::vector<CentralityMeasure_t<GraphType,GeometryType>> list of centrality measure functors 
     */
    template<fishnet::graph::Graph GraphType,typename GeometryType>
    std::vector<CentralityMeasure_t<GraphType,GeometryType>> loadCentralityMeasures() const noexcept {
        std::vector<CentralityMeasure_t<GraphType,GeometryType>> measures;
        for(const auto & [type,description]: centralityMeasuresDescriptions){
            auto optMeasure = fromJson<GraphType,GeometryType>(type,description);
            if(not optMeasure){
                std::cerr << "Could not load centrality measure of type: "+std::string(magic_enum::enum_name(type))+"\n"+description.dump();
                continue;
            }
            measures.push_back(std::move(optMeasure.value()));
        }
        return measures;
    }
};
