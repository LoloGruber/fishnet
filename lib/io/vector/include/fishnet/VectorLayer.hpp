#pragma once
#include <ranges>
#include <utility>
#include <vector>
#include <algorithm>
#include <expected>


#include <fishnet/GeometryObject.hpp>
#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/Shapefile.hpp>

#include <fishnet/GDALInitializer.hpp>
#include <fishnet/GeometryTypeWKBAdapter.hpp>
#include <fishnet/OGRFieldAdapter.hpp>
#include <fishnet/OGRGeometryAdapter.hpp>

#include "FieldType.hpp"
#include "Feature.hpp"


#include <gdal/ogr_spatialref.h>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogr_core.h>

namespace fishnet {

/**
 * Stores the geometries, wrapped in features (which hold the field values)
 * Keeps track of the fields available for the features
 * Stores a SpatialReference
 * @tparam G
 */
template<geometry::GeometryObject G>
class VectorLayer{
private:
    OGRSpatialReference spatialRef;
    std::vector<Feature<G>> features;

    std::unordered_map<std::string,FieldDefinitionVariant> fields;

    using error_type = std::string;

    void addOGRField(OGRFieldDefn * fieldDef,int id){
        std::string fieldName = fieldDef->GetNameRef();
        switch (fieldDef->GetType()){
        case OFTReal:
            fields.emplace(fieldName,FieldDefinition<double>(fieldName,id));
            break;
        case OFTInteger:
             fields.emplace(fieldName,FieldDefinition<int>(fieldName,id));
            break;
        case OFTInteger64:
             fields.emplace(fieldName,FieldDefinition<long long>(fieldName,id));
            break;
        case OFTString:
            fields.emplace(fieldName,FieldDefinition<std::string>(fieldName,id));
            break;
        default:
            break;
        }
    }

    struct AddAttributeVisitor{
        Feature<G> * feature;
        OGRFeature * ogrFeature;

        template<typename T>
        bool operator()(FieldDefinition<T> const & fieldDef) {
            if constexpr(std::same_as<T,int>)
               return feature->addAttribute(fieldDef,ogrFeature->GetFieldAsInteger(fieldDef.getFieldID()));
                
            else if constexpr(std::integral<T>)
                return feature->addAttribute(fieldDef,T(ogrFeature->GetFieldAsInteger64(fieldDef.getFieldID())));
      
            else if constexpr(std::floating_point<T>)
                return feature->addAttribute(fieldDef,T(ogrFeature->GetFieldAsDouble(fieldDef.getFieldID())));
         
            else if constexpr(std::convertible_to<T,std::string>)
                return feature->addAttribute(fieldDef,ogrFeature->GetFieldAsString(fieldDef.getFieldID()));
        }
    };

    explicit VectorLayer(const Shapefile & shapefile){
        GDALInitializer::init();
        if(not shapefile.exists())
            return;
        auto * ds = (GDALDataset *) GDALOpenEx(shapefile.getPath().c_str(), GDAL_OF_VECTOR,nullptr, nullptr,nullptr);
        OGRLayer * layer = ds->GetLayer(0);
        OGRFeatureDefn * layerDef = layer->GetLayerDefn();
        for(int i = 0; i < layerDef->GetFieldCount();i++) {
            addOGRField(layerDef->GetFieldDefn(i),i);
        }
        for(const auto & ogrFeature: layer){
            auto geo = ogrFeature->GetGeometryRef();
            if(geo && wkbFlatten(geo->getGeometryType() == GeometryTypeWKBAdapter::toWKB(G::type))) {
                auto converted = OGRGeometryAdapter::fromOGR<G::type>(*geo);
                if (not converted) 
                    continue;
                Feature f {converted.value()};
                for(const auto & [_,fieldDefinition]: this->fields){
                    std::visit(AddAttributeVisitor(&f,ogrFeature.get()),fieldDefinition);
                }
                addFeature(std::move(f));
            }
        }
        this->spatialRef = *layer->GetSpatialRef()->Clone();
        GDALClose(ds);
    }

    explicit VectorLayer(OGRSpatialReference  spatialReference):spatialRef(std::move(spatialReference)){
        GDALInitializer::init();
    }

    constexpr void writeToDisk(const Shapefile & destination) const noexcept{
        GDALDriver * driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
        destination.remove();
        GDALDataset * outputDataset = driver->Create(destination.getPath().c_str(),0,0,0,GDT_Unknown,0);
        OGRLayer * outputLayer = outputDataset->CreateLayer(destination.getPath().c_str(),this->getSpatialReference().Clone(),GeometryTypeWKBAdapter::toWKB(G::type),0);
        for(const auto & [fieldName,fieldDefinition] :  fields) {
            OGRFieldType fieldType;
            std::visit([&fieldType](auto && fieldVariant){
                using T = typename  std::decay_t<decltype(fieldVariant)>::value_type;
                fieldType = OGRFieldAdapter::fromTypeIndex(typeid(T));
            },fieldDefinition);
            auto fieldDefn = OGRFieldDefn(fieldName.c_str(),fieldType);
            outputLayer->CreateField(&fieldDefn);
        }
        for(const auto & f : this->features){
            auto * feature = new OGRFeature(outputLayer->GetLayerDefn());
            feature->SetGeometry(OGRGeometryAdapter::toOGR(f.getGeometry()).get());

            for(const auto & [fieldName,fieldDefinition]: this->fields){
                std::visit([&fieldName,&f,feature]( auto && var){
                    auto optionalAttribute = f.getAttribute(var);
                    if(optionalAttribute)
                        OGRFieldAdapter::setFieldValue(feature, fieldName, optionalAttribute.value());
                },fieldDefinition);

            }
            OGRErr success = outputLayer->CreateFeature(feature);
            if(success != 0){
                std::cout << "Could not write Geometry: "+f.getGeometry().toString() << std::endl;
            }
        }
        outputLayer->SyncToDisk();
        GDALClose(outputDataset);
    }

    constexpr void remove(util::Predicate<Feature<G>> auto && predicate)noexcept {
        const auto removed = std::ranges::remove_if(this->features,predicate);
        this->features.erase(removed.begin(),removed.end());
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
        return std::views::all(features) | std::views::transform([](const auto & feature){return feature.getGeometry();});
    }

    constexpr util::view_of<Feature<G>> auto getFeatures() const noexcept {
        return std::views::all(features);
    }

    constexpr util::view_of<Feature<G>> auto getFeatures()  noexcept {
        return std::views::all(features);
    }

    constexpr const OGRSpatialReference & getSpatialReference() const noexcept {
        return this->spatialRef;
    }

    constexpr void setSpatialReference(const OGRSpatialReference & spatialReference) noexcept {
        this->spatialRef = spatialReference;
    }

    constexpr void addGeometry(const G & g) noexcept {
        addFeature(Feature<G>(g));
    }

    constexpr void addGeometry(G && g) noexcept {
        addFeature(Feature<G>(std::move(g)));
    }

    constexpr void addAllGeometry(util::forward_range_of<G> auto const & geometries) noexcept {
        std::ranges::for_each(geometries,[this](const auto & g){ addGeometry(g);});
    }

    /**
     * @brief checks if any geometry is equal to the passed geometry
     * WARNING: this method might be slow, since it might compare all geometries of the layer with the passed geometry
     * @param g 
     * @return true 
     * @return false 
     */
    constexpr bool containsGeometry(const G & g) const noexcept {
        return std::ranges::find_if(this->features,[&g](const auto & feature){return feature.getGeometry() == g;}) != std::ranges::end(this->features);
    }


    constexpr void removeGeometry(const G & g) noexcept {
        remove( [&g](const auto & feature){return feature.getGeometry() == g;});
    }

    constexpr void addFeature(Feature<G> && feature) noexcept {
        features.push_back(std::forward<Feature<G>>(feature));
    }

    constexpr void addFeature(const Feature<G> & feature) noexcept {
        features.push_back(feature);
    }

    constexpr bool containsFeature(const Feature<G> & feature) noexcept {
        return std::ranges::contains(this->features, feature);
    }

    constexpr void removeFeature(const Feature<G> & feature) noexcept {
        remove([&feature](const auto & f){return f==feature;});
    }

    template<FieldValueType T>
    [[nodiscard]] constexpr std::expected<FieldDefinition<T>,error_type> addField(const std::string & fieldName) noexcept {
        if(fieldName.length() > 10){
            return std::unexpected("Field name \""+ fieldName+"\" must not exceed a length of 10 characters");
        }
        if(this->fields.contains(fieldName))
            return std::unexpected("Field \"" + fieldName + "\" already exists");
        FieldDefinition<T> field{fieldName};
        this->fields.emplace(fieldName, field);
        return field;
    }

    constexpr std::expected<FieldDefinition<int>,error_type> addIntegerField(const std::string & fieldName) noexcept {
        return addField<int>(fieldName);
    }

    constexpr std::expected<FieldDefinition<double>,error_type> addDoubleField(const std::string & fieldName) noexcept {
        return addField<double>(fieldName);
    }

    constexpr std::expected<FieldDefinition<std::string>,error_type> addTextField(const std::string & fieldName) noexcept {
        return addField<std::string>(fieldName);
    }

    constexpr std::expected<FieldDefinition<size_t>,error_type> addSizeField(const std::string & fieldName) noexcept {
        return addField<size_t>(fieldName);
    }

    constexpr bool hasField(const std::string & fieldName) const noexcept {
        return this->fields.contains(fieldName);
    }

    constexpr void removeField(const std::string & fieldName) noexcept {
        this->fields.erase(fieldName);
    }

    constexpr void clearFields() noexcept {
        fields.clear();
    }

/*    constexpr bool addAttribute(size_t geometryID, const std::string & fieldName, FieldType value){
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
    }*/

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