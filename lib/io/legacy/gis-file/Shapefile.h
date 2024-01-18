//
// Created by grube on 10.01.2022.
//

#ifndef BACHELORARBEIT_SHAPEFILE_H
#define BACHELORARBEIT_SHAPEFILE_H
#include "GISFile.h"
//#include <util/PathHelper.h>
#include <util/PathHelper.h>
#include "utils/Geography/OGRDatasetWrapper.h"
/**
 * Class representing Shapefile inputs
 */
class Shapefile: public GISFile {
private:
    /* Paths to Shapefile-related files*/
    boost::filesystem::path pathToDbf; //database file
    boost::filesystem::path pathToShx; //index file
    boost::filesystem::path pathToPrj; //geographic projection file

public:
    Shapefile(const boost::filesystem::path & pathToShp, WSFTypeEnum type);

    Shapefile(const boost::filesystem::path &pathToShp, const boost::filesystem::path &pathToDbf,
              const boost::filesystem::path &pathToShx, const boost::filesystem::path &pathToPrj,
              WSFTypeEnum type);

    /**
     * Move file(s) to directory root_path
     * @param root_path new directory of the files
     * @param filename
     */
    void move(const boost::filesystem::path &root_path, const std::string &filename);

    /**
     * Copy files to the directory root_path
     * A new Shapefile entity is created!
     * @param root_path
     * @param filename
     * @return Shapefile referencing the shape-files in the copy directory
     */
    std::unique_ptr<Shapefile> copy(const boost::filesystem::path &root_path, const std::string &filename);

    /**
     * Copies Shapefile to Temporary Directory and creates a OGRDataset with said Shapefile
     * @return
     */
    std::unique_ptr<OGRDatasetWrapper> toDataset() override;
};


#endif //BACHELORARBEIT_SHAPEFILE_H
