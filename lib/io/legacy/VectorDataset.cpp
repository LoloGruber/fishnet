// //
// // Created by grube on 15.09.2021.
// //

// #include <iostream>
// #include "OGRDatasetWrapper.h"
// #include "utils/Converter/GeotifftoOGRDataset.h"
// #include "io/File/Shapefile.h"

// GDALDataset * OGRDatasetWrapper::open() {
//     if (not shapefile) {
//         return nullptr;
//     }
//     auto ds = (GDALDataset *) GDALOpenEx(this->shapefile->getPath().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
//     if (!ds) {
//         std::cout << "Could not open ShapeFile: " + this->shapefile->getPath().string() << std::endl;
//     }
//     return ds;
// }

// WSFType OGRDatasetWrapper::getType() {
//     return this->shapefile->getType();
// }

// std::shared_ptr<Shapefile> OGRDatasetWrapper::save(GDALDataset *ds) {
//     ds->GetLayer(0)->SyncToDisk();
//     GDALClose(ds);
//     return this->shapefile;
// }

// OGRSpatialReference *OGRDatasetWrapper::getSpatialRef() {
//     auto ds = open();
//     auto ref = ds->GetLayer(0)->GetSpatialRef();
//     return ref;
// }

// OGRDatasetWrapper::OGRDatasetWrapper(std::unique_ptr<Shapefile> &file) {
//     this->shapefile = std::move(file);
// }

// GDALDataset *OGRDatasetWrapper::newEmpty(boost::filesystem::path &path, WSFTypeEnum typeEnum) {
//     this->shapefile = std::make_shared<Shapefile>(path, typeEnum);
//     auto driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
//     return driver->Create(path.c_str(), 0, 0, 0, GDT_Unknown, nullptr);
// }


// #ifndef BACHELORARBEIT_OGRDATASETWRAPPER_H
// #define BACHELORARBEIT_OGRDATASETWRAPPER_H
// #include <boost/filesystem/path.hpp>
// #include <boost/filesystem/operations.hpp>
// #include "gdal.h"
// #include "gdal_priv.h"

// #include "io/PathHelper.h"
// #include <ogr_core.h>
// #include "ogrsf_frmts.h"
// #include "cpl_conv.h" // for CPLMalloc()
// #include <gdal_alg.h>

// #include "io/File/WSFType.h"
// class Shapefile;
// //#include "Shapefile.h"

// /**
//  * Wrapper class for an OGRDataset
//  */
// class OGRDatasetWrapper {
// private:
//     std::shared_ptr<Shapefile> shapefile; //location of the dataset (stored as shapefile)
// public:
//     explicit OGRDatasetWrapper(std::unique_ptr<Shapefile> & file);

//     OGRDatasetWrapper() = default;

//     /**
//      * Open dataset at the path of the shapefile
//      * @return
//      */
//     GDALDataset *open();

//     /**
//      *
//      * @return Spatial Reference of the dataset located at the path of shapefile
//      */
//     OGRSpatialReference * getSpatialRef();

//     /**
//      * Create a new empty vector dataset
//      * @param path where dataset will be stored
//      * @param typeEnum of the dataset (e.g Network, Imperviousness,..)
//      * @return
//      */
//     GDALDataset * newEmpty(boost::filesystem::path & path, WSFTypeEnum typeEnum);

//     /**
//      * Write Dataset to file
//      * @param ds
//      * @return
//      */
//     std::shared_ptr<Shapefile> save(GDALDataset* ds);

//     WSFType getType();
// };













