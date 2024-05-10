#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include <fishnet/PolygonDistance.hpp>
#include <fishnet/StaticMap.hpp>
#include "FindNeighboursTask.h"

#include "JsonConfigReader.hpp"


using json = nlohmann::json;

struct DistanceBiPredicate{
    double maxDistanceInMeters;

    bool operator()(fishnet::geometry::IPolygon auto const & lhs, fishnet::geometry::IPolygon auto const & rhs) const noexcept {
        auto [l,r] = fishnet::geometry::closestPoints(lhs,rhs);
        return fishnet::WGS84Ellipsoid::distance(l,r) < maxDistanceInMeters;
    }


};


enum class NeighbouringPredicateType{
    ExamplePredicate
};

using namespace std::literals::string_view_literals;
constexpr static std::array<std::pair<std::string_view,NeighbouringPredicateType>,1> predicate_data{{
    {"ExamplePredicate"sv,NeighbouringPredicateType::ExamplePredicate}
}};

constexpr static inline fishnet::util::StaticMap<std::string_view,NeighbouringPredicateType,predicate_data.size()> NEIGHBOURING_PREDICATES {predicate_data};

class NeighboursConfigJsonReader:public BaseJsonConfigReader{
public:
    constexpr static const char * MAX_DISTANCE_KEY = "maxDistanceMeters";
    constexpr static const char * NEIGHBOURING_PREDICATES_KEY = "neighbouring-predicates";
    constexpr static const char * MEMGRAPH_PORT_KEY = "memgraph-port";
    constexpr static const char * MEMGRAPH_HOSTNAME_KEY = "memgraph-host";

    NeighboursConfigJsonReader(const std::string & jsonString):BaseJsonConfigReader(jsonString){}

    NeighboursConfigJsonReader(const std::filesystem::path & pathToConfig):BaseJsonConfigReader(pathToConfig){}

    template<fishnet::geometry::GeometryObject T>
    std::expected<fishnet::util::BiPredicate_t<T>,std::string> fromNeighbouringPredicateType(NeighbouringPredicateType type,const json & predicateDesc) {
        switch(type){
            case NeighbouringPredicateType::ExamplePredicate:
                return std::unexpected("Not implemented yet");
        }
        return std::unexpected("Unexpected NeighbouringPredicate type");
    }
    
    template<typename G>
    void parse(FindNeighboursTask<G> & task) {
        double maxDistance;
        std::string hostname;
        uint16_t port;
        this->config.at(MAX_DISTANCE_KEY).get_to(maxDistance);
        this->config.at(MEMGRAPH_HOSTNAME_KEY).get_to(hostname);
        this->config.at(MEMGRAPH_PORT_KEY).get_to(port);
        task.addNeighbouringPredicate(DistanceBiPredicate(maxDistance))
            .setMaxEdgeDistance(maxDistance)
            .setMemgraphParams(hostname,port);
        for(const auto & neighbourPredicateJson : config.at(NEIGHBOURING_PREDICATES_KEY) ) {
            std::string predicateName;
            neighbourPredicateJson.at("name").get_to(predicateName);
            auto type = NEIGHBOURING_PREDICATES.get(predicateName);
            if(not type) {
                std::cerr << "Neighbouring predicate name \""+predicateName+"\" not supported" << std::endl;
            }
            auto predicate = fromNeighbouringPredicateType<G>(type.value(),neighbourPredicateJson);
            if (not predicate){
                std::cerr << predicate.error() << std::endl;
            }
            task.addNeighbouringPredicate(predicate.value());
        }
        task.indentDescLine("Config: "+config.dump());
    }

};

static_assert(JsonConfigReader<NeighboursConfigJsonReader,FindNeighboursTask<fishnet::geometry::Polygon<double>>>);