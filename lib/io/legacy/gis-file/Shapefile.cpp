//
// Created by grube on 10.01.2022.
//

#include "Shapefile.h"

std::unique_ptr<OGRDatasetWrapper> Shapefile::toDataset() {
    auto copy = this->copy(PathHelper::TMP_PATH(), this->pathToFile.stem().string()); //copy Shapefile first
    return std::make_unique<OGRDatasetWrapper>(copy); //create Dataset with the copy to prevent changes to the original files
}

Shapefile::Shapefile(const boost::filesystem::path &pathToShp, WSFTypeEnum type) : GISFile(pathToShp, type) {
    boost::filesystem::path filename = pathToShp.stem();
    this->pathToPrj = boost::filesystem::path(pathToShp).replace_extension(".prj");
    this->pathToDbf = boost::filesystem::path(pathToShp).replace_extension(".dbf");
    this->pathToShx = boost::filesystem::path(pathToShp).replace_extension(".shx");
}

Shapefile::Shapefile(const boost::filesystem::path &pathToShp, const boost::filesystem::path &pathToDbf,
                     const boost::filesystem::path &pathToShx, const boost::filesystem::path &pathToPrj,
                     WSFTypeEnum type) : GISFile(pathToShp, type) {
    if (!boost::filesystem::exists(pathToDbf)) {
        throw std::invalid_argument(".dbf File: \"" + pathToDbf.string() + "\" not found");
    }
    if (!boost::filesystem::exists(pathToShx)) {
        throw std::invalid_argument(".shx File: \"" + pathToShx.string() + "\" not found ");
    }
    if (!boost::filesystem::exists(pathToPrj)) {
        // no belonging projection found for shape file
        std::cout << "No .prj File found for ShapeFile: " + pathToShp.string() << std::endl;
    }
    this->pathToDbf = pathToDbf;
    this->pathToShx = pathToShx;
    this->pathToPrj = pathToPrj;
}

void Shapefile::move(const boost::filesystem::path &root_path, const std::string &filename) {
    boost::filesystem::path  newShp = root_path / (filename + ".shp");
    boost::filesystem::path  newShx = root_path / (filename + ".shx");
    boost::filesystem::path  newPrj = root_path / (filename + ".prj");
    boost::filesystem::path  newDbf = root_path / (filename + ".dbf");

    try{
        boost::filesystem::rename(this->pathToFile, newShp);
        boost::filesystem::rename(this->pathToDbf, newDbf);
        boost::filesystem::rename(this->pathToShx, newShx);
        this->pathToFile = newShp;
        this->pathToShx = newShx;
        this->pathToDbf = newDbf;

        if (boost::filesystem::exists(this->pathToPrj)) {
            boost::filesystem::rename(this->pathToPrj, newPrj);
            this->pathToPrj = newPrj;
        }

    } catch (const boost::filesystem::filesystem_error &filesystem_error) {
        throw std::invalid_argument("Could not move file(s) to " + root_path.string() + "/" + filename);
    }
}

std::unique_ptr<Shapefile> Shapefile::copy(const boost::filesystem::path &root_path, const std::string &filename){
    boost::filesystem::path  newShp = root_path / (filename + ".shp");
    boost::filesystem::path  newShx = root_path / (filename + ".shx");
    boost::filesystem::path  newPrj = root_path / (filename + ".prj");
    boost::filesystem::path  newDbf = root_path / (filename + ".dbf");

    try{
        boost::filesystem::copy(this->pathToFile, newShp);
        boost::filesystem::copy(this->pathToDbf, newDbf);
        boost::filesystem::copy(this->pathToShx, newShx);
//        this->pathToFile = newShp;
//        this->pathToShx = newShx;
//        this->pathToDbf = newDbf;

        if (boost::filesystem::exists(this->pathToPrj)) {
            boost::filesystem::copy(this->pathToPrj, newPrj);
//            this->pathToPrj = newPrj;
        }

    } catch (const boost::filesystem::filesystem_error &filesystem_error) {
        throw std::invalid_argument("Could not copy file(s) to " + root_path.string() + "/" + filename);
    }
    return std::make_unique<Shapefile>(newShp, newDbf, newShx, newPrj, type.type());
}
