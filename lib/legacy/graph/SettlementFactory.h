//
// Created by grube on 27.09.2021.
//

#ifndef BACHELORARBEIT_SETTLEMENTFACTORY_H
#define BACHELORARBEIT_SETTLEMENTFACTORY_H
#include <list>
#include <future>
#include <util/Geography/OGRDatasetWrapper.h>
#include "graph/vertex/Settlement.h"

/**
 * Factory class for Settlements
 */
class SettlementFactory {
    using DSPointer = const std::unique_ptr<OGRDatasetWrapper> &;
private:
    /**
     * Helper method to get Polygons from WSF-Area layer
     * @param ds WSF-Area dataset
     * @param layerIndex index of polygon layer in GDAL dataset
     * @return vector of polygon objects
     */
    static std::vector<std::unique_ptr<Polygon>> getPolygonsFromLayer(DSPointer ds, int layerIndex = 0);

    /**
     * Helper method to retrieve imperviousness values mapped by the polygon that visualizes the respective value in the dataset
     * @param impDS
     * @param layerIndex
     * @return map: Polygon -> imperviousness
     */
    static std::unordered_map<std::unique_ptr<Polygon>, double> getImperviousnessMapFromLayer(DSPointer impDS, int layerIndex = 0);


    /**
     * Helper method to retrieve population values mapped by the polygon that visualizes the respective value in the dataset
     * @param popDS
     * @param layerIndex
     * @return map: Polygon -> population
     */
    static std::unordered_map<std::unique_ptr<Polygon>, int> getPopulationMapFromLayer(DSPointer popDS, int layerIndex = 0);

public:
    /**
     *
     * @param areaDS WSF-Area Dataset
     * @param impFile WSF-Imp Dataset
     * @param popFile WSF-Pop Dataset
     * @return list of settlements with their outline derived from WSF-Area and attributes from all input datasets
     */
    static std::vector<std::shared_ptr<Settlement>> create(DSPointer areaDS, DSPointer impFile, DSPointer popFile);

    /**
     * Overload function -> calls create(areaDS, null,null)
     * @param areaDS
     * @return
     */
    static std::vector<std::shared_ptr<Settlement>> create(DSPointer areaDS);
};


#endif //BACHELORARBEIT_SETTLEMENTFACTORY_H
