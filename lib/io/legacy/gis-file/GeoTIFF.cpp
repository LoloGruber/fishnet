//
// Created by grube on 09.01.2022.
//

#include "GeoTIFF.h"

std::unique_ptr<VectorDataset> GeoTIFF::toDataset() {
    auto converter = GeotifftoOGRDataset(this->pathToFile, this->type);
    return converter.convert(PathHelper::TMP_PATH());
}


