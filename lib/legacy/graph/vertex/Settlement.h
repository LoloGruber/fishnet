//
// Created by grube on 25.11.2021.
//

#ifndef BACHELORARBEIT_SETTLEMENT_H
#define BACHELORARBEIT_SETTLEMENT_H
#include "geometry/Polygon.h"
#include "graph/vertex/SettlementProperties/SettlementAttributes.h"
#include "graph/vertex/SettlementProperties/SettlementValue.h"
#include "graph/NetworkEntity.h"
class SettlementValue; //forward declaration

/**
 * Class that represents a settlement
 * A settlement is the composition of a polygon marking the outline and the attributes of that settlement
 */
class Settlement : public NetworkEntity{
private:
    std::unique_ptr<Polygon> polygon;
    std::unique_ptr<SettlementAttributes> attributes;
public:
    /* Static Variable to define how the Value of a settlement has to be computed*/
    static std::shared_ptr<SettlementValue> strategy;

    /* Setter for settlement value strategy (not required to use)*/
    static void setSettlementValueStrategy(std::shared_ptr<SettlementValue> & strategy);

    /**
     * Constructor
     * @param polygon storing the outline of the settlement
     * @param attributes storing the attributes of the settlement
     */
    Settlement(std::unique_ptr<Polygon> &polygon, std::unique_ptr<SettlementAttributes> & attributes);

    [[nodiscard]] const std::unique_ptr<SettlementAttributes> &getAttributes() const;

    [[nodiscard]] const std::unique_ptr<Polygon> & getPolygon() const;

    /**
     * Get Polygon as OGRGeometry for Visualization
     * @return
     */
    [[nodiscard]] const std::unique_ptr<OGRGeometry> & getShape() const;

    /**
     * Wrapper for computing the distance between the settlement's polygon to the other's polygon
     * @param other
     * @return
     */
    double distance(Settlement const &other) const;

    /**
     *
     * @return value of settlement according to the SettlementValue strategy
     */
    double value() const;


    /**
     * This Settlement is considered less than the other settlement, when
     * this.value() < other.value()
     * @param other
     * @return
     */
    bool operator <(const Settlement &other)const;

    /**
     * Settlements are considered equal if their outline (Polygon) is equal
     * @param other
     * @return
     */
    bool operator==(const Settlement &other) const;

    /**
     * not (this==other)
     * @param other
     * @return
     */
    bool operator!=(const Settlement &other)const;

    /**
     * Hash Function for SharedPointer of Settlement. Computed on Hash of area and centroid point
     */
    struct Hash{
        std::size_t operator()(const std::shared_ptr<Settlement> &  settlement) const {
            std::size_t areaHash = std::hash<double>()(settlement->getAttributes()->getArea());
            auto centroid = settlement->getPolygon()->centroid();
            std::size_t centroidHash = Vec2D::Hash()(centroid);
            return centroidHash << 2 ^ areaHash;
        };
    };
    /**
     * Equals for SharedPointer of Settlement -> Dereference and compare
     */
    struct Equal{
        bool operator()(const std::shared_ptr<Settlement> & s1, const std::shared_ptr<Settlement> & s2) const{
            return *s1 == *s2;
        };

        bool operator()(std::shared_ptr<Settlement> &  s1, std::shared_ptr<Settlement> &  s2) const{
            return *s1 == *s2;
        };
    };
};


#endif //BACHELORARBEIT_SETTLEMENT_H
