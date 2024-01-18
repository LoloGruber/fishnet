// #include <gdal.h>
// #include <ogr_core.h>
// #include <gdal_priv.h>
// #include "ogrsf_frmts.h"

#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "NearestNeighbours.hpp"
#include "VectorLayer.hpp"
#include "Polygon.hpp"
#include "MultiPolygon.hpp"
#include "PolygonFilter.hpp"
#include "PolygonNearestNeighbours.hpp"

#include "Graph.h"
#include "Contraction.h"

#include "StopWatch.h"

#include "Areaclass.hpp"

// static std::filesystem::path project {"/home/lolo/Documents/fishnet/2022-mp-lorenz-gruber/"}; //todo make invariant of system by getting project root automatically
static std::filesystem::path project {"/home/lolo/Documents/Masterpraktikum/2022-mp-lorenz-gruber/"};

std::vector<double> geometricDistribution(uint8_t classes){
    //median = ln2
    if(classes == 0)return {};
    double geometric = 0.5;
    double boundary = geometric;
    std::vector<double> relativeBoundaries;
    relativeBoundaries.reserve(classes-1);
    for(uint8_t i = 0; i < classes-1 ; i++) {
        relativeBoundaries.push_back(boundary);
        geometric *= geometric;
        boundary = 1-geometric;
    }
    relativeBoundaries.push_back(1);
    return relativeBoundaries;
}

std::optional<fishnet::geometry::SimplePolygon<double>> newVisualizeEdge(const Polygon<double> & from, const Polygon<double> & to) noexcept{
    Segment best = Segment<double>(Vec2D(1,1),Vec2D(1,std::numeric_limits<double>::max()));
    for(const auto s : from.getBoundary().getPoints()){
        for(const auto u: to.getBoundary().getPoints()){
            Segment current {s,u};
            if(current.length() < best.length()){
                best = current;
            }
        }
    }
    double WIDTH = 0.000001;
    auto orthogonalToSegment = best.direction().orthogonal().normalize();
    auto center = best.p() + best.direction().normalize() * (best.length() / 2);
    auto p1 = (best.p() + (orthogonalToSegment * WIDTH / 2));
    auto p2 = (best.p() - (orthogonalToSegment * WIDTH / 2));
    auto p3 = (best.q() + (orthogonalToSegment * WIDTH / 2));
    auto p4 = (best.q() - (orthogonalToSegment * WIDTH / 2));
    if(not best.isValid())
        return std::nullopt;
    /*Comparator to sort the points clockwise*/
    auto cmpClockwise = [&center](Vec2D<double> & u, Vec2D<double> & w) {
        return u.angle(center).getAngleValue() > w.angle(center).getAngleValue();
    };
    std::vector<Vec2D<double>> vectors = {p1, p2, p3, p4};
    std::sort(vectors.begin(), vectors.end(), cmpClockwise);
    try{
        return std::make_optional<SimplePolygon<double>>(vectors);
    }catch(InvalidGeometryException & exc){
        return std::nullopt;
    }
}

std::optional<fishnet::geometry::SimplePolygon<double>> newVisualizeEdge(const MultiPolygon<Polygon<double>> & from, const MultiPolygon<Polygon<double>> & to) noexcept{
    // const Polygon<double> * fromClosest = &(*std::ranges::begin(from.getPolygons())) ;
    // const Polygon<double> * toClosest = &(*std::ranges::begin(to.getPolygons()));
    // double minDistance = std::numeric_limits<double>::max();
    // //TODO take polygon with largest area
    // for(const auto & fromP : from.getPolygons()){
    //     for(const auto & toP: to.getPolygons()){
    //         if(fromP.distance(toP) < minDistance){
    //             fromClosest = &fromP;
    //             toClosest = &toP;
    //             minDistance = fromP.distance(toP);
    //         }
    //     }
    // }
    auto areaComparator = [](const auto & lhs, const auto & rhs){return lhs.area() < rhs.area();};
    auto fromBiggest = std::ranges::max_element(from.getPolygons(),areaComparator);
    auto toBiggest = std::ranges::max_element(to.getPolygons(),areaComparator);
    return newVisualizeEdge(*fromBiggest,*toBiggest);
}

void settlementCategories(fishnet::VectorLayer<Polygon<double>> const & layer, std::filesystem::path const & outputRoot) noexcept {
    std::vector<double> areas;
     for(const auto & p: layer.getGeometries()){
        areas.push_back(p.area());
    }
    std::ranges::sort(areas);
    uint8_t classes = 5;

    std::vector<AreaClass> areaBoundaries;
    uint8_t i = 0;
    for(const auto & b : geometricDistribution(classes)){
        areaBoundaries.push_back(AreaClass(i,areas[size_t(b * double(areas.size())) -1]));
        i++;
    }
    std::map<AreaClass, std::vector<fishnet::geometry::Polygon<double>>> classMap;
    for(const auto & areaClass: areaBoundaries){
        classMap.try_emplace(areaClass,std::vector<fishnet::geometry::Polygon<double>>());
    }

    for( const auto & p : layer.getGeometries()){
        for( auto & [areaClass, polygons] : classMap){
            if(p.area() < areaClass.upperAreaLimit){
                polygons.push_back(p);
                break;
            }
        }
    }
    for(const auto &[areaClass,polygons]:classMap){
        auto name = "_class_"+std::to_string(classes)+"_" + std::to_string(areaClass.id) + ".shp";
        fishnet::Shapefile outFile {std::filesystem::path(outputRoot.string()+name)};
        outFile.remove();
        std::cout << outFile << std::endl;
        auto outLayer = fishnet::VectorLayer<fishnet::geometry::Polygon<double>>::empty(layer.getSpatialReference());
        for(const auto & p : polygons){
            outLayer.addGeometry(p);
        }
        outLayer.write(outFile);
    }

    for(auto largerClassIt = classMap.rbegin(); largerClassIt != classMap.rend(); ){
        AreaClass currentClass = largerClassIt->first;
        ++largerClassIt;
        if(largerClassIt != classMap.rend()){
            using namespace fishnet::geometry;
            std::unordered_map<Vec2D<double>,Polygon<double>> centroidMap;
            std::set<Vec2D<double>,LexicographicOrder> orderedCentroidPoints;
    
            AreaClass nextLargestClass = largerClassIt->first;
            for( const auto & big : classMap.at(currentClass)){
                auto centroid = big.centroid();
                centroidMap.try_emplace(centroid,big);
                orderedCentroidPoints.emplace(centroid);
            }
            std::vector<std::pair<Polygon<double>,Polygon<double>>> edges;
            for(const auto & smaller: classMap.at(nextLargestClass)){
                auto nearest = nearestNeighbour<double>(smaller.centroid(),orderedCentroidPoints);
                edges.push_back(std::make_pair(smaller,centroidMap.at(nearest)));
            }
            fishnet::Shapefile edgeFile {outputRoot.string()+"_edges_"+std::to_string(nextLargestClass.id)+"-"+std::to_string(currentClass.id)+".shp"};
            edgeFile.remove();
            std::cout << edgeFile << std::endl;
            auto edgeLayer = fishnet::VectorLayer<Polygon<double>>::empty(layer.getSpatialReference());
            for(const auto &[from,to]:edges){
                auto edgePolygon = newVisualizeEdge(from,to);
                if(edgePolygon){
                    edgeLayer.addGeometry(Polygon<double>(edgePolygon.value()));
                }
            }
            edgeLayer.write(edgeFile);
        }
    }

}


int main(int argc, char * argv[]){
   
    using namespace fishnet::geometry;
    using namespace fishnet::math;
    std::filesystem::path p = project.parent_path() / std::filesystem::path("data/WSF/WSF3D/Punjab-India/Punjab_shp.shp");
    StopWatch readingInput {"Reading Input"};
    fishnet::Shapefile inputFile {p};
    auto inputLayer = fishnet::VectorLayer<Polygon<double>>::read({p});
    std::filesystem::path outputRootCategories = {project.parent_path() / std::filesystem::path("data/output/prefiltered_settlement_classes/" +p.stem().string())};
    readingInput.stopAndPrint();
    std::vector<Polygon<double>> polygons;


    double requiredArea = 0.000001; //todo use km2
    StopWatch filterClock {"Filtering"};
    polygons = filter(inputLayer.getGeometries(),ContainedOrInHoleFilter(),[requiredArea](const Polygon<double> & p){return p.area() >= requiredArea;});
    filterClock.stopAndPrint();

    // std::vector<Polygon<double>> small;
    // for(int i = 0 ; i < 100; i++){
    //     small.push_back(polygons[i]);
    // }
    // polygons = small;

    fishnet::Shapefile filteredLayerFile {project.parent_path() / std::filesystem::path("data/output/prefiltered_settlements_buffer_edges/") / (inputFile.getPath().stem().string() +"_filtered.shp") };
    filteredLayerFile.remove();
    auto filteredLayer = fishnet::VectorLayer<Polygon<double>>::empty(inputLayer.getSpatialReference());
    filteredLayer.addAllGeometry(polygons);
    StopWatch findEdges {"Neighbours"};
    auto edges = nearestPolygonNeighbours(polygons,2.0);
    findEdges.stopAndPrint();
    fishnet::graph::UndirectedGraph<MultiPolygon<Polygon<double>>> g;
    for(const auto & p: polygons){
        g.addNode(MultiPolygon<Polygon<double>>(p));
    }
    fishnet::Shapefile edgeLayerFile {filteredLayerFile.appendToFilename("_edges")};
    edgeLayerFile.remove();
    auto edgeLayer = fishnet::VectorLayer<SimplePolygon<double>>::empty(filteredLayer.getSpatialReference());
    for(auto && [from,to]:edges){
        auto edge = newVisualizeEdge(from,to);
        if(edge)
            edgeLayer.addGeometry(edge.value());
        g.addEdge({from},{to});
    }
    filteredLayer.write(filteredLayerFile);
    edgeLayer.write(edgeLayerFile);

    double maxBoundingBoxScale = 1.25;
    auto mergePredicate = [&maxBoundingBoxScale](const MultiPolygon<Polygon<double>> & lhs, const MultiPolygon<Polygon<double>> & rhs) {
        auto leftScaled = Rectangle<DEFAULT_NUMERIC>(lhs.aaBB().getPoints()).scale(maxBoundingBoxScale);
        auto rightScaled = Rectangle<DEFAULT_NUMERIC>(rhs.aaBB().getPoints()).scale(maxBoundingBoxScale);
        return leftScaled.crosses(rightScaled) || leftScaled.contains(rightScaled)  || rightScaled.contains(leftScaled);
    };
    auto reduceFunction = [](std::vector<MultiPolygon<Polygon<double>>> const & multiPolygonsVector) {
        // StopWatch singleMerge;
        std::vector<Polygon<double>> allPolygons;
        std::ranges::for_each(multiPolygonsVector | std::views::transform([](const auto & multiPolygon){return multiPolygon.getPolygons();}) | std::views::join,[&allPolygons](const auto & p){allPolygons.push_back(p);});
        return MultiPolygon<Polygon<double>>(std::move(allPolygons),true);
    };
    StopWatch contractionTimer {"Contraction"};
    auto contracted = fishnet::graph::contract(g,mergePredicate,reduceFunction,12);
    contractionTimer.stopAndPrint();
    fishnet::Shapefile contractedVerticesFile = filteredLayerFile.appendToFilename("_contracted");
    fishnet::Shapefile contractedEdgesFile = contractedVerticesFile.appendToFilename("_edges");
    contractedVerticesFile.remove();
    contractedEdgesFile.remove();
    StopWatch visualizationTimer {"Visualization"};
    auto contractedVerticesLayer = fishnet::VectorLayer<MultiPolygon<Polygon<double>>>::empty(filteredLayer.getSpatialReference());
    auto contractedEdgesLayer = fishnet::VectorLayer<SimplePolygon<double>>::empty(filteredLayer.getSpatialReference());
    contractedVerticesLayer.addAllGeometry(contracted.getNodes());
    const std::string areaField = "Area";
    contractedVerticesLayer.addField<double>(areaField);
    for(const auto & [id,polygon]:contractedVerticesLayer.enumerateGeometries()){
        contractedVerticesLayer.addAttribute(id,areaField,polygon.area());
    }
    contractedVerticesLayer.write(contractedVerticesFile);
    for(const auto & edge:contracted.getEdges()){
        auto visualizedEdge = newVisualizeEdge(edge.getFrom(),edge.getTo());
        if(visualizedEdge)
            contractedEdgesLayer.addGeometry(visualizedEdge.value());
    }
    contractedEdgesLayer.write(contractedEdgesFile);
    visualizationTimer.stopAndPrint();
    return 0;
}