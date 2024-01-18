//
// Created by grube on 09.01.2022.
//

#ifndef BACHELORARBEIT_GEOTIFF_H
#define BACHELORARBEIT_GEOTIFF_H
#include "GISFile.h"
#include "../converters/GeotifftoOGRDataset.h"
#include "../VectorDataset.h"
/**
 * Class representing Geotiff inputs
 */
class GeoTIFF : public GISFile {
public:
    GeoTIFF(const boost::filesystem::path &path, WSFTypeEnum typeEnum): GISFile(path,typeEnum){};

    /**
     * Geotiff have to be Converter to Shapefiles first! Store converted Shapefile in Temporary Directory and return OGRDataset from said Shapefile
     * @return
     */
    std::unique_ptr<VectorDataset> toDataset() override;

};


#endif //BACHELORARBEIT_GEOTIFF_H
