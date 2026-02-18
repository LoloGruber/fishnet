#pragma once
#include <expected>
#include "Shapefile.hpp"
#include "GeoTiff.hpp"
#include <filesystem>
#include <gdal/gdal.h>
#include <gdal_alg.h>
#include <ogrsf_frmts.h>
#include <gdal/ogr_core.h>
#include <gdal/gdal_priv.h>
#include <fishnet/TemporaryDirectiory.h>

namespace fishnet{
class GISConverter {
public:
    /**
     * @brief Tries to convert a GeoTiff to Shp file
     * 
     * @param geoTiff source geoTiff file
     * @param maskZero see: GDALPolygonize(...) 
     * @param showProgress show progress in console
     * @return std::expected<Shapefile,std::string>: Shapefile on success, otherwise string explaining the error
     */
    static std::expected<Shapefile,std::string> convert(const GeoTiff & geoTiff,bool maskZero = true, bool showProgress=false) noexcept{
        GDALAllRegister();
        GDALDataset *src = (GDALDataset *) GDALOpen(geoTiff.getPath().c_str(), GA_ReadOnly);
        if(src == nullptr)
            return std::unexpected("Could not open Geotiff-Dataset: "+geoTiff.getPath().string());
        auto driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
        if(driver == nullptr)
            return std::unexpected("No suitable ESRI Shapefile driver detected");
        std::filesystem::path conversionOutputDir = std::filesystem::current_path() / "conversion_output";
        std::filesystem::create_directories(conversionOutputDir);
        std::filesystem::path destPath = conversionOutputDir / geoTiff.getPath().stem().replace_extension(".shp");
        GDALDataset *dest = driver->Create(destPath.c_str(), 0,0, 0, GDT_Unknown,nullptr);
        auto spatialReference = src->GetSpatialRef();
        OGRLayer * layer = dest->CreateLayer(destPath.stem().c_str(),spatialReference->Clone(),wkbPolygon, nullptr);
        const char *fieldName = "pixel_val";
        OGRFieldDefn fieldDefn = OGRFieldDefn(fieldName, OFTInteger);
        if(layer->CreateField(&fieldDefn)!=OGRERR_NONE)
            return std::unexpected("Could not create field for pixel values");
        int fieldID = layer->GetLayerDefn()->GetFieldIndex(fieldName);
        char ** papszOptions = nullptr;
        papszOptions = CSLSetNameValue(papszOptions, "8CONNECTED", "8");
        GDALPolygonize(
            src->GetRasterBand(1),
            maskZero?src->GetRasterBand(1):nullptr,
            layer,
            fieldID,
            papszOptions,
            showProgress?GDALTermProgress:nullptr,
            nullptr
        );
        layer->SyncToDisk();
        GDALClose(src);
        GDALClose(dest);
        return Shapefile(destPath);
    }
};
}
