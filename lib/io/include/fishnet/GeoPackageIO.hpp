#pragma once
#include <fishnet/IOConcepts.hpp>
#include <fishnet/GeoPackage.hpp>

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
class GeoPackageReader {
private:
    std::vector<std::string> gdalOpenOptions;
    bool checked = true;
public:
    using geometry_type = G;
    using file_type = GeoPackage;

    GeoPackageReader() = default;

    GeoPackageReader(fishnet::util::forward_range_of<std::string> auto && openOptions, bool checked) : checked(checked) {
        for(auto && opt : openOptions) {
            this->gdalOpenOptions.push_back(std::move(opt));
        }
    }

    void setChecked(bool checked) {
        this->checked = checked;
    }

    Either<VectorLayer<G>,std::string> operator()(const GeoPackage & geopackage) const {
        GDALInitializer::init();
        if(not geopackage.exists())
            return std::unexpected("GeoPackage does not exist, could not read from File: \"" + geopackage.getPath().string() + "\"");
        std::vector<const char*> openOptionsVec;
        for (const auto& opt : gdalOpenOptions) {
            openOptionsVec.push_back(opt.c_str());
        }
        openOptionsVec.push_back(nullptr);
        const char** openOptions = openOptionsVec.data();
        auto * ds = (GDALDataset *) GDALOpenEx(geopackage.getPath().c_str(), GDAL_OF_VECTOR, nullptr, openOptions, nullptr);
        if(ds == nullptr)
            return std::unexpected("Could not open GeoPackage: \"" + geopackage.getPath().string() + "\" with GDAL");
        auto layer = OGRLayerAdapter<G>::fromOGR(ds->GetLayer(0), checked);
        GDALClose(ds);
        return layer;
    }
};

template<geometry::GeometryObject G>
class GeoPackageWriter {
private:
    bool overwrite = false;
    std::vector<std::string> options;
public:
    GeoPackageWriter() = default;

    GeoPackageWriter(bool overwrite) : overwrite(overwrite) {}

    GeoPackageWriter(bool overwrite, fishnet::util::forward_range_of<std::string> auto && options) : overwrite(overwrite) {
        for(auto && opt : options) {
            this->options.push_back(std::move(opt));
        }
    }

    Either<GeoPackage,std::string> operator()(const VectorLayer<G> & layer, const GeoPackage & output) const {
        GDALInitializer::init();
        GDALDriver * driver = GetGDALDriverManager()->GetDriverByName("GPKG");
        if (driver == nullptr) {
            return std::unexpected("Could not find GDAL driver for GeoPackage");
        }
        output.remove();
        GDALDataset * outputDataset = driver->Create(output.getPath().c_str(), 0, 0, 0, GDT_Unknown, nullptr);
        if (outputDataset == nullptr) {
            return std::unexpected("Could not create GeoPackage dataset: \"" + output.getPath().string() + "\"");
        }
        const char * const createOptions[] = {nullptr};
        OGRLayer * outputLayer = outputDataset->CreateLayer(
            output.getPath().stem().c_str(),
            layer.getSpatialReference().Clone(),
            GeometryTypeWKBAdapter::toWKB(G::type),
            const_cast<char **>(createOptions)
        );
        if (outputLayer == nullptr) {
            GDALClose(outputDataset);
            return std::unexpected("Could not create layer in GeoPackage: \"" + output.getPath().string() + "\"");
        }
        auto result = OGRLayerAdapter<G>::toOGR(layer, outputLayer);
        outputLayer->SyncToDisk();
        GDALClose(outputDataset);
        return result.transform([&output](const auto & _) {
            return output;
        });
    }
};

static_assert(VectorLayerReader<GeoPackageReader<geometry::Polygon<double>>, GeoPackage, geometry::Polygon<double>>, "GeoPackageReader must satisfy VectorLayerReader concept");
static_assert(VectorLayerWriter<GeoPackageWriter<geometry::Polygon<double>>, geometry::Polygon<double>, GeoPackage>, "GeoPackageWriter must satisfy VectorLayerWriter concept");

} // namespace fishnet
