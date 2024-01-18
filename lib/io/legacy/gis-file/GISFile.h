//
// Created by grube on 09.01.2022.
//

#ifndef BACHELORARBEIT_GISFILE_H
#define BACHELORARBEIT_GISFILE_H
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "type/WSFType.h"
#include "../VectorDataset.h"

//#include "OGRDatasetWrapper.h"
class VectorDataset;

/**
 * Base class to define the behavior of geographic input files
 */
class GISFile{
protected:
    boost::filesystem::path pathToFile;
    WSFType type;
public:
    GISFile(const boost::filesystem::path & path, WSFTypeEnum typeEnum){
//        if (!boost::filesystem::exists(path)) {
//            throw std::invalid_argument("GISFile: \""+ path.string()+" not found");
//        }
        this->pathToFile = path;
        type = WSFType(typeEnum);
    }

    /**
     * Return dataset from stored file path. Conversions (to Shapefile) might be necessary
     * @return The dataset that is stored within the file
     */
    virtual std::unique_ptr<VectorDataset> toDataset() = 0;

    boost::filesystem::path getPath();

    WSFType getType();
};



#endif //BACHELORARBEIT_GISFILE_H
