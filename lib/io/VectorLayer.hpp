#pragma once
#include <ranges>
#include <vector>
#include <algorithm>
#include <typeindex>

#include "GeometryObject.hpp"
#include "CollectionConcepts.hpp"
#include "Shapefile.hpp"
#include "GDALInitializer.hpp"

#include "FieldType.hpp"
#include "GeometryTypeWKBAdapter.hpp"
#include "OGRFieldAdapter.hpp"
#include "OGRGeometryAdapter.hpp"
#include "AlternativeKeyMap.hpp"
#include "NestedMap.hpp"

#include "Feature.hpp"

#include <ogr_spatialref.h>
#include <gdal.h>
#include <gdal_priv.h>
#include <ogr_core.h>

namespace fishnet {

struct FieldID{
    size_t value;
    constexpr operator size_t() {
        return this->value;
    }
    constexpr inline bool operator==(const FieldID & other) const noexcept {
        return this->value == other.value;
    }
}; 
}

namespace std {
    template<>
    struct hash<fishnet::FieldID>{
        constexpr inline size_t operator()(const fishnet::FieldID & fieldID) const noexcept {
            return fieldID.value;
        }
    };
}

namespace fishnet {

template<geometry::GeometryObject G>
class VectorLayer{
private:
    OGRSpatialReference spatialRef;
    size_t objectCounter = 0;
    std::vector<std::pair<size_t,G>> objects;


    util::NestedMap<size_t,FieldID,FieldType> attributes; // (geometryID,FieldID) -> Value 

    size_t fieldCounter = 0;
    util::AlternativeKeyMap<FieldID,std::string,std::type_index> fields;

    constexpr static const char * FISHNET_ID_FIELD_NAME = "FISHNET_ID";

    VectorLayer(const Shapefile & shapefile){
        GDALInitializer::init();
        if(not shapefile.exists())
            return;
        GDALDataset * ds = (GDALDataset *) GDALOpenEx(shapefile.getPath().c_str(), GDAL_OF_VECTOR,0,0,0);
        OGRLayer * layer = ds->GetLayer(0);
        for(const auto & feature: layer){
            auto geo = feature->GetGeometryRef();
            if(geo && wkbFlatten(geo->getGeometryType() == GeometryTypeWKBAdapter::toWKB(G::type))) {
                auto converted = OGRGeometryAdapter::fromOGR<G::type>(*geo);
                if (converted) {
                    addGeometry(converted.value());
                }
            }
        }
        this->spatialRef = *layer->GetSpatialRef()->Clone();
        GDALClose(ds);
    }

    VectorLayer(const OGRSpatialReference & spatialReference):spatialRef(spatialReference){
        GDALInitializer::init();
    }

    constexpr void writeToDisk(const Shapefile & destination) const noexcept{
        GDALDriver * driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
        destination.remove();
        GDALDataset * outputDataset = driver->Create(destination.getPath().c_str(),0,0,0,GDT_Unknown,0);
        OGRLayer * outputLayer = outputDataset->CreateLayer(destination.getPath().c_str(),this->getSpatialReference().Clone(),GeometryTypeWKBAdapter::toWKB(G::type),0);
        auto uidFieldDef = OGRFieldDefn(FISHNET_ID_FIELD_NAME,OFTInteger64);
        outputLayer->CreateField(&uidFieldDef);
        for(const auto & [fieldID, typeInfo]: fields) {
            auto fieldDefinition = OGRFieldDefn(fields.getAlternative(fieldID).value_or("UNDEFINED").c_str(),OGRFieldAdapter::fromTypeIndex(typeInfo));
            outputLayer->CreateField(&fieldDefinition);
        }
        for(const auto & [id,g] : this->objects){
            OGRFeature * feature = new OGRFeature(outputLayer->GetLayerDefn());
            feature->SetGeometry(OGRGeometryAdapter::toOGR(g).get());
            feature->SetField(FISHNET_ID_FIELD_NAME,GIntBig(id));
            for(const auto & [fieldID, value] : this->attributes.innerMap(id)){
                
            }
            OGRErr success = outputLayer->CreateFeature(feature);
            if(success != 0){
                std::cout << "Could not write Geometry: "+g.toString() << std::endl;
            }
        }
        outputLayer->SyncToDisk();
        GDALClose(outputDataset);
    }

public: 

    static VectorLayer<G> empty(const OGRSpatialReference & spatialReference){
        return VectorLayer(spatialReference);
    }

    static VectorLayer<G> read(const Shapefile & shapefile) {
        if(not shapefile.exists())
            throw std::invalid_argument("Shapefile does not exists, could not read from File");
        return VectorLayer(shapefile);
    }

    constexpr util::view_of<G> auto getGeometries() const noexcept {
        return std::views::all(objects) | std::views::transform([](const auto & pair){return pair.second;});
    }

    constexpr util::view_of<std::pair<size_t,G>> auto enumerateGeometries() const noexcept {
        return std::views::all(objects);
    }

    constexpr const OGRSpatialReference & getSpatialReference() const noexcept {
        return this->spatialRef;
    }

    constexpr void setSpatialReference(const OGRSpatialReference & spatialReference) noexcept {
        this->spatialRef = spatialReference;
    }

    constexpr size_t addGeometry(const G & g) noexcept {
        auto id = objectCounter++;
        objects.emplace_back(std::make_pair(id,g));
        attributes.try_insert(id);
        return id;
    }

    /**
     * @brief returns the geomety object associated with the id
     * WARNING: this method might be slow, since it checks the ids of all geometries for a match
     * @param id 
     * @return  std::optional<const G &> 
     */
    constexpr std::optional<const G &> getGeometry(size_t id) const noexcept {
        auto result = std::ranges::find_if(objects,[id](const auto & geometryPair){return id == geometryPair.first;});
        if(result != std::ranges::end(objects))
            return std::make_optional<const G &>(result->second);
        return std::nullopt;
    }

    constexpr util::view_of<size_t> auto addAllGeometry(util::forward_range_of<G> auto const & geometries) noexcept {
        std::ranges::for_each(geometries,[this](const G & g){ addGeometry(g);});
        return std::views::all(objects) | std::views::transform([](const auto & pair){return pair.first;});
    }

    /**
     * @brief checks if any geometry is equal to the passed geometry
     * WARNING: this method might be slow, since it might compare all geometries of the layer with the passed geometry
     * @param g 
     * @return true 
     * @return false 
     */
    constexpr bool containsGeometry(const G & g) const noexcept {
        return std::ranges::find_if(this->objects,[&g](const auto & pair){return pair.second == g;}) != std::ranges::end(this->objects);
    }

    constexpr bool containsGeometry(size_t id) const noexcept {
        return this->attributes.containsOuterKey(id);
    }

    constexpr void removeGeometry(const G & g) noexcept {
        auto removePredicate = [&g](const auto & objPair){return objPair.second == g;};
        const auto removed = std::ranges::remove_if(this->objects,removePredicate);
        this->attributes.eraseOuterKey(std::ranges::begin(removed)->first);
        this->objects.erase(removed.begin(),removed.end());
    }

    template<FieldValueType T>
    constexpr bool addField(const std::string & fieldName) noexcept {
        if(fieldName.length() > 10){
            std::cerr << "Field name \""+ fieldName+"\" must not exceed a length of 10 characters";
            return false;
        }
        if(this->fields.containsAlternative(fieldName) || fieldName == FISHNET_ID_FIELD_NAME)
            return false;
        auto success = this->fields.insert({fieldCounter},fieldName,typeid(T));
        if (success)
            fieldCounter++;
        return success;
    }

    constexpr bool addIntegerField(const std::string & fieldName) noexcept {
        return addField<int>(fieldName);
    }

    constexpr bool addDoubleField(const std::string & fieldName) noexcept {
        return addField<double>(fieldName);
    }

    constexpr bool addTextField(const std::string & fieldName) noexcept {
        return addField<std::string>(fieldName);
    }

    constexpr bool addSizeField(const std::string & fieldName) noexcept {
        return addField<size_t>(fieldName);
    }

    constexpr bool hasField(const std::string & fieldName) const noexcept {
        return this->fields.containsAlternative(fieldName);
    }

    constexpr void removeField(const std::string & fieldName) noexcept {
        auto removedFieldID = this->fields.getKey(fieldName);
        if (not removedFieldID or not this->fields.eraseFromKey(removedFieldID.value()))
            return;
        this->attributes.eraseInnerKey(removedFieldID.value());
    }

    constexpr void clearFields() noexcept {
        fields.clear();
        std::ranges::for_each(attributes,[this](const auto & idAttributePair){idAttributePair.second.clear();});
    }

    constexpr bool addAttribute(size_t geometryID, const std::string & fieldName, FieldType value){
        auto expectedType = fields.getOrElse(fieldName,typeid(nullptr));
        auto actualType = std::type_index(std::visit( [](auto&&x)->decltype(auto){ return typeid(x);}, value ));
        if (expectedType != actualType){
            return false;
        }
        if(not this->fields.contains(fieldName))
             return false;
        if(not this->attributes.containsOuterKey(geometryID))
             return false;
        return this->attributes.try_insert(geometryID,fields.getKey(fieldName).value(),value);
    }

    template<FieldValueType ResultType>
    constexpr std::optional<ResultType> getAttribute(size_t geometryID, const std::string & fieldName) const noexcept {
        if(not this->attributes.containsOuterKey(geometryID) || fieldName.empty() || not fields.getKey(fieldName))
            return std::nullopt;
        auto optVal =  this->attributes.get(geometryID,fields.getKey(fieldName).value());
        if(not optVal){
            return std::nullopt;
        }
        try{
            return std::get<ResultType>(optVal.value());
        }catch(std::bad_variant_access const & ex){
            return std::nullopt;
        }
    }

    constexpr std::optional<double> getAttributeAsDouble(size_t geometryID, const std::string & fieldName) const noexcept {
        return getAttribute<double>(geometryID,fieldName);
    }

    constexpr std::optional<int> getAttributeAsInteger(size_t geometryID, const std::string & fieldName) const noexcept {
        return getAttribute<int>(geometryID,fieldName);
    }

    constexpr std::optional<std::string> getAttributeAsText(size_t geometryID, const std::string & fieldName) const noexcept {
        return getAttribute<std::string>(geometryID,fieldName);
    }

    constexpr std::optional<size_t> getAttributeAsSizeType(size_t geometryID, const std::string & fieldName) const noexcept {
        return getAttribute<size_t>(geometryID,fieldName);
    }


    constexpr bool removeAttribute(size_t geometryID, const std::string & fieldName) {
        if(not this->fields.contains(fieldName)) return false;
        if(not this->attributes.contains(geometryID)) return false;
        this->attributes.erase(geometryID,this->fields.getKey(fieldName).value());
        return true;
    }

    constexpr void write(const Shapefile & destination) const noexcept {
        if(destination.exists()){
            this->writeToDisk(destination.incrementFileVersion());
        }else {
             this->writeToDisk(destination);
        }
    }

    constexpr void overwrite(const Shapefile & destination) const noexcept {
        this->writeToDisk(destination);
    }

};
}