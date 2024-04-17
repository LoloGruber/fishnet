//
// Created by grube on 27.09.2021.
//

#include "SettlementFactory.h"
#include <gdal_priv.h>
#include "gdal.h"
#include <ogr_core.h>
#include "cpl_conv.h"
#include "ogrsf_frmts.h"
#include <iostream>
#include "glob.h"
#include "io/ProgressPrinter.h"
#include <future>
#include <thread>
std::vector<std::unique_ptr<Polygon>> SettlementFactory::getPolygonsFromLayer(DSPointer ds, int layerIndex) {
    std::vector<std::unique_ptr<Polygon>> polygons;
    auto gdalds = ds->open()->GetLayer(layerIndex); //open dataset on layer <layerIndex>
    auto progress = ProgressPrinter((long) gdalds->GetFeatureCount(), "Getting Polygons from WSFArea Layer");
    for (auto &feature: ds->open()->GetLayer(layerIndex)) {
        std::unique_ptr<Polygon> current = Polygon::create(*feature); //multipolygons are merged here
        if (current) {
            polygons.push_back(std::move(current));
        }
        progress.visit();
    }
    return polygons;
}

std::unordered_map<std::unique_ptr<Polygon>, double>
SettlementFactory::getImperviousnessMapFromLayer(DSPointer impDS, int layerIndex) {
    if (impDS == nullptr) {
        return {};
    }
    std::unordered_map<std::unique_ptr<Polygon>, double> impMap;
    for (auto &feature: impDS->open()->GetLayer(layerIndex)) {
        auto impPolygonCollection = Polygon::createFromMultiPolygon(*feature); //split up multipolygon in this case and process each polygon individually
        int index = feature->GetFieldIndex(impDS->getType().string().c_str()); //get index of field that stores the Imperviousness value
        if (index == -1) {
            index = 0;
        }
        for (auto &impPolygon: impPolygonCollection) {
            if (impPolygon) {
                double imperviousness = feature->GetFieldAsInteger(index) / 100.0; //must be divided by 100 to scale down from 1-100 to 1% to 100%
                impMap.insert(std::make_pair(std::move(impPolygon), imperviousness));
            }
        }
    }
    return impMap;
}

std::unordered_map<std::unique_ptr<Polygon>,int>
SettlementFactory::getPopulationMapFromLayer(DSPointer popDS, int layerIndex) {
    if (popDS == nullptr) {
        return {};
    }
    std::unordered_map<std::unique_ptr<Polygon>, int> popMap;
    for (auto &feature: popDS->open()->GetLayer(layerIndex)) {
        auto popPolygonCollection = Polygon::createFromMultiPolygon(*feature); //split up multipolygon in this case and process each polygon individually
        int index = feature->GetFieldIndex(popDS->getType().string().c_str());//get index of field that stores the population value
        if (index == -1) {
            index = 0;
        }
        for (auto & popPolygon : popPolygonCollection) {
            if (popPolygon) {
                int population = feature->GetFieldAsInteger(index) / 100; //divide by 100 to retrieve actual population value
                popMap.insert(std::make_pair(std::move(popPolygon), population));
            }
        }
    }
    return popMap;
}



std::vector<std::shared_ptr<Settlement>> SettlementFactory::create(DSPointer areaDS, DSPointer impDS, DSPointer popDS) {
    if (areaDS->getType().type() != WSFTypeEnum::AREA or (impDS != nullptr and impDS->getType().type() != WSFTypeEnum::IMPERVIOUSNESS) or
            (popDS != nullptr and popDS->getType().type() != WSFTypeEnum::POPULATION)) {
        std::cerr << "Type of supplied files does not match the method description! Creating no settlements"
                  << std::endl;
        return {};
    }
    if (areaDS == nullptr) {
        std::cerr << "Area Dataset is null! Creating no settlements"
                  << std::endl;
        return {};
    }

    // Get all Polygons from the Area dataset
    auto polygonsFuture = std::async(std::launch::async, getPolygonsFromLayer,std::ref( areaDS), 0);

    // Get all Polygons from Imp dataset
    auto impMapFuture = std::async(std::launch::async, getImperviousnessMapFromLayer, std::ref(impDS), 0);

    //Get all Polygons from Pop dataset
    auto popMapFuture = std::async(std::launch::async, getPopulationMapFromLayer, std::ref(popDS), 0);


    auto polygons = polygonsFuture.get();
    auto impMap = impMapFuture.get();
    auto popMap = popMapFuture.get();

    auto settlementCount = polygons.size();
    std::vector<std::shared_ptr<Settlement>> settlements;
    auto progress = ProgressPrinter(settlementCount,"Finding Imperviousness and Population values for "+ std::to_string(settlementCount) + " settlements");
    for (auto &current: polygons) {
        double area = current->area();
        double imp = -1;
        int pop = 0;

        // Loop over Polygons in Imperviousness Dataset -> find shapes located within the current polygon of the area dataset
        // update imperviousness with weighted average regarding the covered area
        auto impThread = std::thread([&impMap, &current, &area, &imp] {
            std::vector<std::unordered_map<std::unique_ptr<Polygon>, double>::iterator> visitedImp;
            for (auto it = impMap.begin(); it != impMap.end(); it++) {
                if (current->contains(*it->first)) {
                    if (imp == -1) {
                        imp = it->second;
                    } else {
                        imp = imp * (area - it->first->area()) / area + (it->first->area()/area * it->second);
                    }
                    visitedImp.push_back(it);
                }
            }
            for (auto &v: visitedImp) {
                impMap.erase(v);
            }
        });


        //Loop over Shapes in Population Dataset -> find shapes located within the current polygon-> add population to population count for the current settlement
        auto popThread = std::thread([&popMap, &current, &pop] {
             std::vector<std::unordered_map<std::unique_ptr<Polygon>, int>::iterator> visitedPop;
             for (auto it = popMap.begin(); it != popMap.end(); it++) {
                 if (current->contains(*it->first)) {
                     pop += it->second;
                     visitedPop.push_back(it);
                 }
             }

             for (auto &v: visitedPop) {
                 popMap.erase(v);
             }
         });

        popThread.join();
        impThread.join();
        /* Create Attribute entity for the related to the current Polygon */
        auto attributes = std::make_unique<SettlementAttributes>(area, imp, pop);
        /* Composite Polygon (Outline) and Attributes to a Settlement*/
        settlements.push_back(std::make_shared<Settlement>(current, attributes));
        progress.visit();
    }
    return settlements;
}

std::vector<std::shared_ptr<Settlement>> SettlementFactory::create(const std::unique_ptr<OGRDatasetWrapper> &areaDS) {
    return SettlementFactory::create(areaDS, nullptr, nullptr);
}




