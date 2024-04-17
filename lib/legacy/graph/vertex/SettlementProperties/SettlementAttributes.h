//
// Created by grube on 06.01.2022.
//

#ifndef BACHELORARBEIT_SETTLEMENTATTRIBUTES_H
#define BACHELORARBEIT_SETTLEMENTATTRIBUTES_H

#include <ogr_feature.h>
#include <list>
#include "WSFSettlementType.h"
#include "visualization/EntityField.h"
#include <ogr_core.h>
#include "ogrsf_frmts.h"
#include "graph/centrality/CentralityEntry/CentralityEntry.h"

/**
 * Class storing attributes of a settlement separately,
 * containing simple getter and initializer for the export into an ogrfeature
 */
class SettlementAttributes {
private:
    double area;
    double imperviousness;
    int population;
    WSFSettlementType type;



public:
    /* Field names for the export to an ogrfeature*/
    static const char * popString;
    static const char *impString;
    static const char *areaString;
    static const char *entityString;

    SettlementAttributes(double area, double imperviousness, int population);

    static bool isValidArea(double area);

    static bool isValidImperviousness(double imp);

    static bool isValidPopulation(int pop);

    [[nodiscard]] double getArea() const;

    [[nodiscard]] double getImperviousness() const;

    [[nodiscard]] int getPopulation() const;

    [[nodiscard]] WSFSettlementType getType() const;

    /**
     * Create fields with names corresponding to the defined static const char * above
     * @param layer the fields will be created on
     */
    static void createFields(OGRLayer * layer);

    /**
     * Set Attributes of this settlement attribute entity for the given OGRFeature
     * @param feature
     */
    void setFields(OGRFeature * feature) const;
};



#endif //BACHELORARBEIT_SETTLEMENTATTRIBUTES_H
