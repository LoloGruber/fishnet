#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/MemgraphAdjacency.hpp>
#include <fishnet/GISFile.hpp>
#include <fishnet/VectorIO.hpp>

/**
 * @brief Settlement Object, which fulfills the requirements of a Polygon and for a Node stored in the Memgraph DB.
 * 
 * @tparam S specific shape type (e.g. Polygon<double>)
 */
template<fishnet::geometry::Shape S>
class SettlementShape:public S{
private:
    size_t id; // unique id of the settlement
    FileReference fileRef; // file, which stores the shape
public:
    template<typename... Args>
    SettlementShape(size_t id, FileReference fileRef, Args&&... args):S(std::forward<Args>(args)...),id(id),fileRef(std::move(fileRef)){}
    size_t key() const noexcept {
        return id;
    }

    const FileReference & file() const noexcept {
        return fileRef;
    }

    bool operator==(const SettlementShape<S> & other) const noexcept {
        return this->key() == other.key();
    }

    static std::vector<SettlementShape<S>> load(
        fishnet::util::range_of<fishnet::AbstractVectorFile> auto const & files,
        std::convertible_to<MemgraphAdjacency<SettlementShape<S>>> auto & adj
        fishnet::util::Consumer<fishnet::VectorLayer<S>> auto && onRead = fishnet::util::NOOP{}
    ){
        std::vector<SettlementPolygon<P>> polygons;
        std::vector<std::string> inputStrings;
        std::ranges::for_each(this->inputs,[&inputStrings](auto const & file){inputStrings.push_back(file.getPath().filename().string());});
        this->desc["inputs"]=inputStrings;
        for(const auto & shp : inputs) {
            auto layer = fishnet::VectorIO::read<P>(shp);
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
};