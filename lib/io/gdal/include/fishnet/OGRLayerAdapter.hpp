#pragma once
#include <fishnet/VectorLayer.hpp>
#include <fishnet/OGRGeometryAdapter.hpp>
#include <gdal/gdal.h>
#include "ogrsf_frmts.h"
#include <gdal/ogr_core.h>
#include <gdal/gdal_priv.h>
#include "OGRFieldAdapter.hpp"

namespace fishnet {

/**
 * @brief Reads and writes a VectorLayer whose geometries are backed by the OGR data source
 *
 * Nothing is rebuilt out of the data source on the way in, and nothing is rebuilt point by point on
 * the way out. @see fishnet::geometry::OGRLayerGeometry for the geometries this covers, and the
 * adapters' toNative() for getting a fishnet value type back where one is needed.
 */
template<geometry::OGRWritableGeometry G>
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
     * @brief WKB type an output layer holding G has to be created with
     */
    static OGRwkbGeometryType layerGeometryType() {
        if constexpr (fishnet::geometry::DynamicGeometry<G>)
            return wkbUnknown; // takes whatever its features happen to hold
        else
            return fishnet::geometry::GeometryTypeWKBAdapter::toWKB(G::type);
    }

    /**
     * @brief Converts an OGRLayer to a fishnet::VectorLayer
     * 
     * @param ogrLayer pointer to the OGRLayer
     * @return util::Either<VectorLayer<G>, std::string> VectorLayer if successful, error message otherwise
     * @note features whose geometry is missing, or of a type a layer of G cannot hold, are skipped
     */
    static Either<VectorLayer<G>, std::string> fromOGR(OGRLayer * ogrLayer)
    requires geometry::OGRLayerGeometry<G> {
        if(ogrLayer == nullptr)
            return std::unexpected("Could not read from OGRLayer, pointer is null");
        VectorLayer<G> layer {};
        OGRFeatureDefn * layerDef = ogrLayer->GetLayerDefn();
        for(int i = 0; i < layerDef->GetFieldCount();i++) {
            addOGRField(layer, layerDef->GetFieldDefn(i),i);
        }
        auto addFeatureIfPresent = [&layer](std::optional<G> && geometry, OGRFeature * ogrFeaturePtr) -> void{
            if (not geometry)
                return;
            Feature<G> f {std::move(geometry.value())};
            for(const auto & [_,fieldDefinition]: layer.getFieldsMap()){
                std::visit(AddAttributeVisitor(&f,ogrFeaturePtr),fieldDefinition);
            }
            layer.addFeature(std::move(f));
        };
        for(const auto & ogrFeature: ogrLayer){
            if (ogrFeature->GetGeometryRef() == nullptr)
                continue;
            // The geometry is taken off the feature rather than copied out of it: the feature is
            // destroyed at the end of this iteration, so borrowing it would leave the layer holding
            // dangling pointers. Which geometries a layer of G accepts is narrowTo's business.
            fishnet::geometry::OGRGeometryAdapter geometry {
                fishnet::geometry::OGRUniquePtr<OGRGeometry>(ogrFeature->StealGeometry())};
            addFeatureIfPresent(std::move(geometry).template narrowTo<G>(), ogrFeature.get());
        }
        if (auto * spatialRef = ogrLayer->GetSpatialRef(); spatialRef != nullptr)
            layer.setSpatialReference(*spatialRef);
        return layer;
    }
    /**
     * @brief Converts a fishnet::VectorLayer to an OGRLayer
     * 
     * @param layer vector layer to be converted    
     * @param outputLayer inout parameter, should be already created with the correct geometry type and spatial reference
     * @return util::Either<OGRLayer, std::string> OGRLayer if successful, error message otherwise
     */
    static Either<OGRLayer *, std::string> toOGR(const VectorLayer<G> & layer, OGRLayer * outputLayer){
        for(const auto & [fieldName,fieldDefinition] :  layer.getFieldsMap()) {
            OGRFieldType fieldType;
            // get OGRFieldType from FieldDefinition<T> type -> T
            std::visit([&fieldType](auto && fieldVariant){
                using T = typename  std::decay_t<decltype(fieldVariant)>::value_type;
                fieldType = OGRFieldAdapter::fromTypeIndex(typeid(T));
            },fieldDefinition);
            auto fieldDefn = OGRFieldDefn(fieldName.c_str(),fieldType);
            fieldDefn.SetPrecision(20);
            std::ignore = outputLayer->CreateField(&fieldDefn); // add OGRFieldDefinition to output layer, ignore return value since it is not needed
        }
        for(const auto & f : layer.getFeatures()){
            auto * feature = new OGRFeature(outputLayer->GetLayerDefn());
            // the geometry is backed by an OGR geometry already, so it is handed over as it is
            // instead of being rebuilt point by point
            if constexpr (fishnet::geometry::DynamicGeometry<G>) {
                // a type erased geometry needs no inspection at all, whatever it holds is written
                feature->SetGeometry(f.getGeometry().raw());
            } else if constexpr (G::type == fishnet::geometry::GeometryType::POINT) {
                OGRPoint point {f.getGeometry().getX(), f.getGeometry().getY()};
                feature->SetGeometry(&point);
            } else if constexpr (std::same_as<G, fishnet::geometry::OGRRingAdapter>) {
                // a ring adapter wraps its ring in a shell polygon, the ring is its boundary
                feature->SetGeometry(f.getGeometry().raw()->getExteriorRing());
            } else if constexpr (fishnet::geometry::OGRLayerGeometry<G>) {
                feature->SetGeometry(f.getGeometry().raw()); // already backed by an OGR geometry
            } else if constexpr (fishnet::geometry::IRing<G>) {
                // a fishnet geometry is converted through the adapter of its own kind, which is
                // the same conversion the adapters offer everywhere else
                fishnet::geometry::OGRRingAdapter adapted {f.getGeometry()};
                feature->SetGeometry(adapted.raw()->getExteriorRing());
            } else if constexpr (fishnet::geometry::IPolygon<G>) {
                fishnet::geometry::OGRPolygonAdapter adapted {f.getGeometry()};
                feature->SetGeometry(adapted.raw());
            } else if constexpr (fishnet::geometry::IMultiPolygon<G>) {
                fishnet::geometry::OGRMultiPolygonAdapter adapted {f.getGeometry()};
                feature->SetGeometry(adapted.raw());
            } else {
                static_assert(false, "toOGR has no case for this geometry: a type was added to "
                    "OGRWritableGeometry without saying how it is turned into an OGR geometry");
            }

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