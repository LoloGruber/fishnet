#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/GISFile.hpp>
#include <fishnet/VectorIO.hpp>
#include <fishnet/Task.hpp>
#include <fishnet/FileReference.hpp>
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

    auto geometry() const noexcept {
        return static_cast<S>(*this);
    }

    bool operator==(const SettlementShape<S> & other) const noexcept {
        return this->key() == other.key();
    }

    template<fishnet::VectorGISFile F,fishnet::util::Predicate<S> Filter = fishnet::util::TruePredicate>
    static std::vector<SettlementShape<S>> read(
        fishnet::util::range_of<F> auto const & files,
        fishnet::VectorLayerReader<F,S> auto && reader,
        fishnet::util::UnaryFunction<F,FileReference> auto &&  fileRefMapper,
        const Filter & filter  = fishnet::util::TruePredicate {},
        const std::string & idLayerName = Task::FISHNET_ID_FIELD
    ){
        std::vector<SettlementShape<S>> settlements;
        OGRSpatialReference spatialRef;
        static_assert(fishnet::VectorGISFile<std::ranges::range_value_t<decltype(files)>>, "Files must be VectorGISFiles");
        for(const auto & shp : files) {
            auto layer = reader(shp).value_or_throw();
            if(layer.isEmpty())
                continue;
            if(spatialRef.IsEmpty())
                spatialRef = layer.getSpatialReference();
            if(not spatialRef.IsSame(&layer.getSpatialReference()))
                throw std::runtime_error("Spatial reference of files do not match!\nExpecting: "+std::string(spatialRef.GetName())+"\nActual: "+layer.getSpatialReference().GetName());
            FileReference fileRef = fileRefMapper(shp);
            auto fishnetIDField = layer.getSizeField(idLayerName).value_or_throw("Could not find FISHNET_ID field in shp file: \n"+shp.getPath().string());
            for(const auto & feature : layer.getFeatures()) {
                auto id = feature.getAttribute(fishnetIDField).value_or_throw("Could not find FISHNET_ID attribute in feature with geometry:\n"+ feature.getGeometry().toString());
                if(filter(feature.getGeometry()))
                    settlements.emplace_back(id,fileRef,std::move(feature.getGeometry()));
            }   
        }
        return settlements;        
    }

    template<fishnet::VectorGISFile F, fishnet::util::Predicate<S> Filter = fishnet::util::TruePredicate>
    static std::vector<SettlementShape<S>> read(
        const F & file,
        fishnet::VectorLayerReader<F,S> auto && reader,
        fishnet::util::UnaryFunction<F,FileReference> auto &&  fileRefMapper,
        const Filter & filter  = fishnet::util::TruePredicate {},
        const std::string & idLayerName = Task::FISHNET_ID_FIELD
    ){
        return read<F,Filter>(std::views::single(file),std::forward<decltype(reader)>(reader),std::forward<decltype(fileRefMapper)>(fileRefMapper),filter,idLayerName);
    }
};


namespace std {
    template<fishnet::geometry::Shape S>
    struct hash<SettlementShape<S>> {
        size_t operator()(const SettlementShape<S> & settlement) const noexcept {
            return std::hash<size_t>()(settlement.key());
        }
    };
}