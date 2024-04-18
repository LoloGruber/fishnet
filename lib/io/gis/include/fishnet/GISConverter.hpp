//
// Created by lolo on 18.04.24.
//
#pragma once
#include "Shapefile.hpp"
#include "GeoTiff.hpp"
#include <gdal/gdal.h>
#include <gdal_alg.h>
#include <gdal/gdal_priv.h>
#include <fishnet/TemporaryDirectiory.h>

namespace fishnet{
class GISConverter {
public:
    static std::expected<Shapefile,std::string> convert(const GeoTiff & geoTiff,bool maskZero = true, bool showProgress=true) noexcept{
        GDALDataset *src = (GDALDataset *) GDALOpen(geoTiff.getPath().c_str(), GA_ReadOnly);
        if(src == nullptr)
            return std::unexpected("Could not open Geotiff-Dataset: "+geoTiff.getPath().string());
        auto driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
        if(driver == nullptr)
            return std::unexpected("No suitable ESRI Shapefile driver detected");
        std::filesystem::path destPath = geoTiff.getPath().parent_path() / geoTiff.getPath().stem().replace_extension(".shp");
        GDALDataset *dest = driver->Create(destPath.c_str(), 0,0, 0, GDT_Unknown,nullptr);
        auto spatialReference = src->GetSpatialRef();
        OGRLayer * layer = dest->CreateLayer(destPath.stem().c_str(),spatialReference->Clone(),wkbPolygon, nullptr);
        const char *fieldName = "pixel_value";
        OGRFieldDefn fieldDefn = OGRFieldDefn(fieldName, OFTInteger);
        if(layer->CreateField(&fieldDefn)!=OGRERR_NONE)
            return std::unexpected("Could not create field for pixel values");
        int fieldID = layer->GetLayerDefn()->GetFieldIndex(fieldName);
        char ** papszOptions = nullptr;
        papszOptions = CSLSetNameValue(papszOptions, "8CONNECTED", "8");
        if(maskZero) {
            if (showProgress) {
                GDALPolygonize(src->GetRasterBand(1), src->GetRasterBand(1), layer, fieldID, papszOptions,
                               GDALTermProgress,
                               nullptr);
            } else {
                GDALPolygonize(src->GetRasterBand(1), src->GetRasterBand(1), layer, fieldID, papszOptions,
                               nullptr,
                               nullptr);
            }

        } else {
            if (showProgress) {
                GDALPolygonize(src->GetRasterBand(1), nullptr, layer, fieldID, papszOptions,
                               GDALTermProgress,
                               nullptr);
            } else {
                GDALPolygonize(src->GetRasterBand(1), nullptr, layer, fieldID, papszOptions,
                               nullptr,
                               nullptr);
            }
        }
        layer->SyncToDisk();
        GDALClose(src);
        GDALClose(dest);
        return Shapefile(destPath);
    }
};
}
