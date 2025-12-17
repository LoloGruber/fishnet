#pragma once
#include <fishnet/IOConcepts.hpp>
#include <fishnet/Shapefile.hpp>

#include <fishnet/GDALInitializer.hpp>
#include <fishnet/GeometryTypeWKBAdapter.hpp>
#include <fishnet/OGRFieldAdapter.hpp>
#include <fishnet/OGRGeometryAdapter.hpp>
#include <fishnet/OGRLayerAdapter.hpp>

#include <gdal/ogr_spatialref.h>
#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogr_core.h>

namespace fishnet {

template<geometry::GeometryObject G>
class ShapefileReader {
private:
    constexpr static std::array<const char *, 1> DEFAULT_OPEN_OPTIONS = { "ADJUST_TYPE=YES"};
    std::vector<std::string> gdalOpenOptions;
public:
    using geometry_type = G;
    using file_type = Shapefile;

    ShapefileReader() {
        for(const auto & opt : DEFAULT_OPEN_OPTIONS) {
            this->gdalOpenOptions.push_back(opt);
        }
    }

    ShapefileReader(fishnet::util::forward_range_of<std::string> auto && openOptions) {
        for(auto && opt : openOptions) {
            this->gdalOpenOptions.push_back(std::move(opt));
        }
    }

    util::Either<VectorLayer<G>,std::string> operator()(const Shapefile & shapefile) const {
        GDALInitializer::init();
        if(not shapefile.exists())
            return std::unexpected("Shapefile does not exists, could not read from File: \"" + shapefile.getPath().string() + "\"");
        // Prepare GDAL open options
        std::vector<const char*> openOptionsVec;
        for (const auto& opt : gdalOpenOptions) {
            openOptionsVec.push_back(opt.c_str());
        }
        openOptionsVec.push_back(nullptr);
        const char** openOptions = openOptionsVec.data();
        auto * ds = (GDALDataset *) GDALOpenEx(shapefile.getPath().c_str(), GDAL_OF_VECTOR,nullptr, openOptions,nullptr);
        auto layer = OGRLayerAdapter<G>::fromOGR(ds->GetLayer(0));
        GDALClose(ds);
        return layer;
    }
};

template<geometry::GeometryObject G>
class ShapefileWriter { 
private:
    bool overwrite = false;
    std::vector<std::string> options;
public:
    ShapefileWriter() = default;

    ShapefileWriter(bool overwrite) : overwrite(overwrite) {}

    ShapefileWriter(bool overwrite, fishnet::util::forward_range_of<std::string> auto && options) : overwrite(overwrite) {
        for(auto && opt : options) {
            this->options.push_back(std::move(opt));
        }
    }
    util::Either<Shapefile,std::string> operator()(const VectorLayer<G> & layer, const Shapefile & output) const {
        GDALInitializer::init();
        GDALDriver * driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
        if (driver == nullptr) {
            return std::unexpected("Could not find GDAL driver for ESRI Shapefile");
        }
        output.remove(); // delete already existing files, if present
        GDALDataset * outputDataset = driver->Create(output.getPath().c_str(),0,0,0,GDT_Unknown,0);
        const char * const options[] = {"SPATIAL_INDEX=YES",nullptr};
        OGRLayer * outputLayer = outputDataset->CreateLayer(output.getPath().c_str(),layer.getSpatialReference().Clone(),GeometryTypeWKBAdapter::toWKB(G::type),const_cast<char **>(options));
        // for(const auto & [fieldName,fieldDefinition] :  layer.getFieldsMap()) {
        //     OGRFieldType fieldType;
        //     // get OGRFieldType from FieldDefinition<T> type -> T
        //     std::visit([&fieldType](auto && fieldVariant){
        //         using T = typename  std::decay_t<decltype(fieldVariant)>::value_type;
        //         fieldType = OGRFieldAdapter::fromTypeIndex(typeid(T));
        //     },fieldDefinition);
        //     auto fieldDefn = OGRFieldDefn(fieldName.c_str(),fieldType);
        //     fieldDefn.SetPrecision(20);
        //     outputLayer->CreateField(&fieldDefn); // add OGRFieldDefinition to output layer
        // }
        // for(const auto & f : layer.getFeatures()){
        //     auto * feature = new OGRFeature(outputLayer->GetLayerDefn());
        //     feature->SetGeometry(OGRGeometryAdapter::toOGR(f.getGeometry()).get());

        //     for(const auto & [fieldName,fieldDefinition]: layer.getFieldsMap()){
        //         // visitor to set attributes for OGRFeature
        //         std::visit([&fieldName,&f,feature]( auto && var){
        //             auto optionalAttribute = f.getAttribute(var);
        //             if(optionalAttribute)
        //                 OGRFieldAdapter::setFieldValue(feature, fieldName, optionalAttribute.value());
        //         },fieldDefinition);

        //     }
        //     OGRErr success = outputLayer->CreateFeature(feature);
        //     if(success != 0){
        //         std::cerr << "Could not write Geometry: "+f.getGeometry().toString() << std::endl;
        //     }
        // }
        auto result = OGRLayerAdapter<G>::toOGR(layer,outputLayer);
        outputLayer->SyncToDisk();
        GDALClose(outputDataset);
        // return result.transform([&output](const auto & _) {
        //     return output;
        // });
        return output;
    }
};

static_assert(VectorLayerReader<ShapefileReader<geometry::Polygon<double>>, Shapefile, geometry::Polygon<double>>, "ShapefileReader must satisfy VectorLayerReader concept");
static_assert(VectorLayerWriter<ShapefileWriter<geometry::Polygon<double>>, geometry::Polygon<double>, Shapefile>, "ShapefileWriter must satisfy VectorLayerWriter concept");
} // namespace fishnet