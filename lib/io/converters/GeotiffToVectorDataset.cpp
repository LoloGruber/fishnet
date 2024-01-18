//
// Created by grube on 15.09.2021.
//
#include <cpl_string.h>
#include <gdal_priv.h>
#include "../gis-file/type/WSFType.h"
#include "VectorDatasetConverter.h"
#include "io/PathHelper.h"
#include "gdal.h"
#include "gdal_alg.h"
#include "ogrsf_frmts.h"
#include "GeotiffToVectorDataset.h"
#include "io/File/Shapefile.h"
#include "io/ProgressPrinter.h"


//GeotiffToShapeConverter::GeotiffToShapeConverter(boost::filesystem::path & pathSrc, bool maskZero, bool showProgress, char ** options, WSFInputType & type){
//    this->pathSrc = pathSrc;
//    this->maskZero = maskZero;
//    this->showProgress = showProgress;
//    this->options = options;
//    this->type = type;
//}

//GeotiffToShapeConverter::GeotiffToShapeConverter(boost::filesystem::path &pathSrc, WSFInputType & type){
//    this->pathSrc = pathSrc;
//    this->maskZero = true;
//    this->showProgress = true;
//    char **papszOptions = nullptr;
//    papszOptions = CSLSetNameValue( papszOptions, "8CONNECTED", "8" );
//    this->options = papszOptions;
//    this->type = type;
//}

//GeotiffToShapeConverter::GeotiffToShapeConverter(boost::filesystem::path &pathSrc) {
//    this->pathSrc = pathSrc;
//    this->maskZero = true;
//    this->showProgress = true;
//    char **papszOptions = nullptr;
//    papszOptions = CSLSetNameValue( papszOptions, "8CONNECTED", "8" );
//    this->options = papszOptions;
//    this->type = WSFInputType(WSFInputTypeEnum::OTHER);
//}

OGRFieldDefn *GeotiffToVectorDataset::constructOGRField() {
    auto ogrField = new OGRFieldDefn(type.string().c_str(), type.datatype());
    return ogrField;
}

std::unique_ptr<VectorDataset> GeotiffToVectorDataset::convert() {
        boost::filesystem::path tmp = PathHelper::TMP_PATH(); //Store at in temporary directory by default
        return convert(tmp);
}
    /**
     *
     * @param save_path_root directory where the resulting Shapefile(s) will be stored. => save_path_root /  stem_of_src_geotiff. shp
     * @return ShapeFile Wrapper to simplify the handling of shapefiles
     */
std::unique_ptr<VectorDataset> GeotiffToVectorDataset::convert(const boost::filesystem::path &save_path_root){
        auto progress = ProgressPrinter("Convert " + this->pathSrc.string() + " to Shapefile");
        GDALDataset *srcDataset;
        srcDataset = (GDALDataset *) GDALOpen(this->pathSrc.c_str(), GA_ReadOnly);
        if (srcDataset == nullptr) {
            throw std::invalid_argument("Could not open Geotiff-Dataset: " + this->pathSrc.string());
        }

        GDALDataset *dstDataset;
        auto driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
        if (driver == nullptr) {
            throw std::invalid_argument("No suitable driver detected");
        }

         /*Create Vector dataset*/
        dstDataset = driver->Create(save_path_root.c_str(), 0, 0, 0, GDT_Unknown, nullptr);

        auto spatialReference = srcDataset->GetSpatialRef(); //get SpatialReference of source
        boost::filesystem::path finalShpFile = (save_path_root / this->pathSrc.stem()).replace_extension(".shp");

        /* Create new Layer of polygons in Vector dataset with spatial reference of source dataset and at path <root_path>/<Geotiff-Filename>.shp */
        OGRLayer *layer = dstDataset->CreateLayer(finalShpFile.stem().c_str(),
                                                  const_cast<OGRSpatialReference *>(spatialReference), wkbPolygon, nullptr);


        OGRFieldDefn *fieldDefn = this->constructOGRField(); //Create field to store the pixel values
        if (fieldDefn!= nullptr && layer->CreateField(this->constructOGRField()) != OGRERR_NONE) { // add field to vector layer
            throw std::logic_error(
                    "Could not create Field for type \"" + this->type.string() + "\" and destination file \"" +
                    finalShpFile.string() + "\"");
        }

        int id = layer->GetLayerDefn()->GetFieldIndex(this->type.string().c_str()); //index of new field, usually 0

        /* Call GDALPolygonize with the specified parameters*/
        if(maskZero) {
            if (showProgress) {
                GDALPolygonize(srcDataset->GetRasterBand(1), srcDataset->GetRasterBand(1), layer, id, this->options,
                               GDALTermProgress,
                               nullptr);
            } else {
                GDALPolygonize(srcDataset->GetRasterBand(1), srcDataset->GetRasterBand(1), layer, id, this->options,
                               nullptr,
                               nullptr);
            }

        } else {
            if (showProgress) {
                GDALPolygonize(srcDataset->GetRasterBand(1), nullptr, layer, id, this->options,
                               GDALTermProgress,
                               nullptr);
            } else {
                GDALPolygonize(srcDataset->GetRasterBand(1), nullptr, layer, id, this->options,
                               nullptr,
                               nullptr);
            }
        }
        layer->SyncToDisk();
        GDALClose(dstDataset);
        GDALClose(srcDataset);
        auto shapefile = std::make_unique<Shapefile>(finalShpFile, this->type.type());
        return std::make_unique<VectorDataset>(shapefile); //get OGRDataset from the converted Shapefile
}





