//
// Created by grube on 10.01.2022.
//
#include "GISFile.h"

boost::filesystem::path GISFile::getPath() {
    return this->pathToFile;
}

WSFType GISFile::getType() {
    return this->type;
}
