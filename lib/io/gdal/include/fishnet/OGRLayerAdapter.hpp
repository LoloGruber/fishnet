#include <fishnet/VectorLayer.hpp>
#include <gdal/gdal.h>
#include <gdal/ogr_core.h>
#include <gdal/gdal_priv.h>

#include "OGRGeometryAdapter.hpp"

namespace fishnet {

template<geometry::GeometryObject G>
class OGRLayerAdapter {
private:
    /**
     * @brief Adaptor function the add fishnet fields to the layer from a OGRFieldDefinition
     * 
     * @param fieldDef pointer to the OGRFieldDefinition
     * @param id field ID for the FieldDefinition
     */
    static void addOGRField(VectorLayer<G> & layer,OGRFieldDefn * fieldDef,int id){
        std::string fieldName = fieldDef->GetNameRef();
        switch (fieldDef->GetType()){
        case OFTReal:
            layer.addDoubleField(fieldName,id);
            break;
        case OFTInteger:
             layer.addIntegerField(fieldName,id);
            break;
        case OFTInteger64:
             layer.addSizeField(fieldName,id);
            break;
        case OFTString:
            layer.addTextField(fieldName,id);
            break;
        default:
            break;
        }
    }


    /**
     * @brief Variant-Visitor to add attributes to features when reading a shapefile
     * 
     */
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
public:
    /**
     * @brief Converts an OGRLayer to a fishnet::VectorLayer
     * 
     * @param ogrLayer pointer to the OGRLayer
     * @return util::Either<VectorLayer<G>, std::string> VectorLayer if successful, error message otherwise
     */
    static util::Either<VectorLayer<G>, std::string> fromOGR(OGRLayer * ogrLayer){
        if(ogrLayer == nullptr)
            return std::unexpected("Could not read from OGRLayer, pointer is null");
        VectorLayer<G> layer {};
        OGRFeatureDefn * layerDef = ogrLayer->GetLayerDefn();
        for(int i = 0; i < layerDef->GetFieldCount();i++) {
            addOGRField(layer, layerDef->GetFieldDefn(i),i);
        }
        for(const auto & ogrFeature: ogrLayer){
            auto geo = ogrFeature->GetGeometryRef();
            if constexpr(G::type == fishnet::geometry::GeometryType::MULTIPOLYGON){
                if(geo && wkbFlatten(geo->getGeometryType()) == GeometryTypeWKBAdapter::toWKB(G::polygon_type::type)) {
                    auto converted = OGRGeometryAdapter::fromOGR<G::polygon_type::type>(*geo);
                    if (not converted) 
                        continue;
                    Feature<G> f {{converted.value()}};
                    for(const auto & [_,fieldDefinition]: layer.getFieldsMap()){
                        std::visit(AddAttributeVisitor(&f,ogrFeature.get()),fieldDefinition);
                    }
                    layer.addFeature(std::move(f));
                }                
            }
            if(geo && wkbFlatten(geo->getGeometryType()) == GeometryTypeWKBAdapter::toWKB(G::type)) {
                auto converted = OGRGeometryAdapter::fromOGR<G::type>(*geo);
                if (not converted) 
                    continue;
                Feature<G> f {converted.value()};
                for(const auto & [_,fieldDefinition]: layer.getFieldsMap()){
                    std::visit(AddAttributeVisitor(&f,ogrFeature.get()),fieldDefinition);
                }
                layer.addFeature(std::move(f));
            }
        }
        layer.setSpatialReference(*ogrLayer->GetSpatialRef()->Clone());
        return layer;
    }
    /**
     * @brief Converts a fishnet::VectorLayer to an OGRLayer
     * 
     * @param layer vector layer to be converted    
     * @param outputLayer inout parameter, should be already created with the correct geometry type and spatial reference
     * @return util::Either<OGRLayer, std::string> OGRLayer if successful, error message otherwise
     */
    static util::Either<OGRLayer *, std::string> toOGR(const VectorLayer<G> & layer, OGRLayer * outputLayer){
        for(const auto & [fieldName,fieldDefinition] :  layer.getFieldsMap()) {
            OGRFieldType fieldType;
            // get OGRFieldType from FieldDefinition<T> type -> T
            std::visit([&fieldType](auto && fieldVariant){
                using T = typename  std::decay_t<decltype(fieldVariant)>::value_type;
                fieldType = OGRFieldAdapter::fromTypeIndex(typeid(T));
            },fieldDefinition);
            auto fieldDefn = OGRFieldDefn(fieldName.c_str(),fieldType);
            fieldDefn.SetPrecision(20);
            outputLayer->CreateField(&fieldDefn); // add OGRFieldDefinition to output layer
        }
        for(const auto & f : layer.getFeatures()){
            auto * feature = new OGRFeature(outputLayer->GetLayerDefn());
            feature->SetGeometry(OGRGeometryAdapter::toOGR(f.getGeometry()).get());

            for(const auto & [fieldName,fieldDefinition]: layer.getFieldsMap()){
                // visitor to set attributes for OGRFeature
                std::visit([&fieldName,&f,feature]( auto && var){
                    auto optionalAttribute = f.getAttribute(var);
                    if(optionalAttribute)
                        OGRFieldAdapter::setFieldValue(feature, fieldName, optionalAttribute.value());
                },fieldDefinition);

            }
            OGRErr success = outputLayer->CreateFeature(feature);
            if(success != 0){
                return std::unexpected("Could not write Geometry: "+f.getGeometry().toString());
            }
        }
        return outputLayer;
        
    }
};
}