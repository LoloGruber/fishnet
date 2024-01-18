#ifndef VectorDataset_H
#define VectorDataset_H
#include <memory>
#include "gis-file/Shapefile.h"
#include "gdal.h"
#include <ogr_core.h>
#include "geometry/Polygon.h"
class VectorDataset
{
private:
    std::shared_ptr<Shapefile> shapefile;
public:
    explicit VectorDataset(std::unique_ptr<Shapefile> & file);
    VectorDataset() = default;
    ~VectorDataset();

    OGRSpatialReference * getSpatialRef();
    GDALDataset * open();
    std::vector<std::unique_ptr<Polygon>> getPolygonsFromLayer(int layerIndex=0);
    std::unordered_map<std::unique_ptr<Polygon, int> getPolygonsWithValue(int layerIndex=0);
};



#endif
