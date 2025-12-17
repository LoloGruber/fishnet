//
// Created by grube on 05.01.2022.
//

#include "NetworkVisualization.h"
#include "io/ProgressPrinter.h"
#include <ogr_featurestyle.h>



std::shared_ptr<Shapefile> NetworkVisualization::create(boost::filesystem::path &outputPath) {
    auto datasetWrapper = OGRDatasetWrapper();
    auto outDS = datasetWrapper.newEmpty(outputPath,WSFTypeEnum::NETWORK); //create new empty dataset of type network
    OGRLayer *layer = outDS->CreateLayer(outputPath.c_str(), spatialRef, wkbPolygon, nullptr); //create new polygon layer
    /* Initialize EntityField and CentralityMeasure fields*/
    EntityField::init(layer);
    initCentralityFields(layer);
    visualize(this->network->getSettlements(), layer);
    visualize(this->network->getEdges(), layer);
    return datasetWrapper.save(outDS);
}


void NetworkVisualization::visualize(const Network::SettlementSet &settlements, OGRLayer *layer) {
    auto featureDefn = layer->GetLayerDefn();
    SettlementAttributes::createFields(layer); // create all fields for the settlement attributes
    auto progess = ProgressPrinter(settlements.size(),"Visualizing "+ std::to_string(settlements.size()) +" settlements");
    for (auto &settlement: settlements) {
        auto feature = new OGRFeature(featureDefn); //new Feature for the current settlement
        feature->SetGeometry(settlement->getShape().get()); //set outline of settlement as shape of the feature
        settlement->getAttributes()->setFields(feature); //set attributes of the settlement as fields of the feature
        setCentralityFields(settlement->getCentralityValues(), feature); //set centrality entries of settlement as fields of the feature
        //setCentralityFieldsOfFeature(settlement->getCentralityValues(), layer, &feature);
        auto success = layer->CreateFeature(feature);
        progess.visit();
    }
}

void NetworkVisualization::visualize(const Network::EdgeSet &edges, OGRLayer *layer) {
    auto featureDefn = layer->GetLayerDefn();
//    OGRStyleTable ogrStyleTable;
//    auto *styleMgr = new OGRStyleMgr(&ogrStyleTable);
//    auto *brush = new OGRStyleBrush();
//    brush->SetForeColor("#000000");
//    styleMgr->AddPart(brush); // https://gdal.org/user/ogr_feature_style.html
    auto progess = ProgressPrinter(edges.size(),"Visualizing "+ std::to_string(edges.size()) +" edges");
    for (auto &edge: edges) {
        auto geom = edge->getShape();
        if (geom == nullptr) {
            continue;
        }
        auto feature = new OGRFeature(featureDefn);
        edge->getWeight()->setField(layer, feature, *edge); //set weight of edges as field of feature


        setCentralityFields(edge->getCentralityValues(),feature); //set centrality entries of edge as fields of feature
        feature->SetGeometry(geom); //set outline of edge as shape of the feature
        auto success = layer->CreateFeature(feature);
        progess.visit();
    }
}

void NetworkVisualization::initCentralityFields(OGRLayer *layer) {
    for (auto &c: centralities) {
        c->createField(layer);
    }
}

void NetworkVisualization::setCentralityFields(const std::vector<std::unique_ptr<CentralityEntry>> &entries, OGRFeature * feature){
    for (auto &e: entries) {
        e->setField(feature);
    }
}

