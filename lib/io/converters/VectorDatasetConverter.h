//
// Created by grube on 15.09.2021.
//

#ifndef BACHELORARBEIT_VectorDatasetConverter
_H
#define BACHELORARBEIT_VectorDatasetConverter
_H
#include "../VectorDataset.h"
/**
 * Interface to convert a file to a OGRDataset (necessary for Geotiff files)
 */
class VectorDatasetConverter
 {

public:
    virtual std::unique_ptr<OGRDatasetWrapper> convert(const boost::filesystem::path &save_path_root) = 0;
    virtual std::unique_ptr<OGRDatasetWrapper> convert() = 0;

    virtual ~VectorDatasetConverter
    () = default;
};


#endif //BACHELORARBEIT_VectorDatasetConverter
_H
