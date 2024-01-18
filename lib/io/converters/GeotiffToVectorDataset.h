//
// Created by grube on 21.09.2021.
//

#ifndef BACHELORARBEIT_GeotiffToVectorDataset
_H
#define BACHELORARBEIT_GeotiffToVectorDataset
_H

#include "VectorDatasetConverter.h"
#include "io/File/WSFType.h"
#include <memory>
/**
 * Converter for Geotiff to Shapefiles and ultimately a OGRDataset
 */
class GeotiffToVectorDataset
: public VectorDatasetConverter {
private:
    /* Parameters for the GDAL::polygonize functions stored as attributes*/
    boost::filesystem::path pathSrc;
    bool maskZero;
    bool showProgress;
    char **options;
    WSFType type;

    /**
     * Construct field to store the former pixel value
     * @return
     */
    OGRFieldDefn *constructOGRField();
public:
    GeotiffToVectorDataset
    (const boost::filesystem::path &pathSrc, bool maskZero, bool showProgress, char **options, WSFType & type): pathSrc(pathSrc), maskZero(maskZero), showProgress(showProgress), options(options),
                                                                                                                                   type(type){} ;

    GeotiffToVectorDataset
    (const boost::filesystem::path &pathSrc, WSFType & type): pathSrc(pathSrc), maskZero(true), showProgress(true), type(type){
        char **papszOptions = nullptr;
        papszOptions = CSLSetNameValue(papszOptions, "8CONNECTED", "8");
        this->options = papszOptions;
    };

//    explicit GeotiffToShapeConverter(boost::filesystem::path &pathSrc);
    std::unique_ptr<OGRDatasetWrapper> convert(const boost::filesystem::path &save_path_root) override;

    std::unique_ptr<OGRDatasetWrapper> convert() override;

};
#endif //BACHELORARBEIT_GeotiffToVectorDataset
_H
