#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include "MemgraphAdjacency.hpp"

/**
 * @brief Wrapper Object, which fulfills the requirements of a Polygon and for a Node stored in the Memgraph DB
 * 
 * @tparam P specific polygon type (e.g. Polygon<double>)
 */
template<fishnet::geometry::IPolygon P>
class SettlementPolygon:public P{
private:
    size_t id; // unique id of the settlement
    FileReference fileRef; // file, which stores the polygon
public:
    template<typename... Args>
    SettlementPolygon(size_t id, FileReference fileRef, Args&&... args):P(std::forward<Args>(args)...),id(id),fileRef(std::move(fileRef)){}
    size_t key() const noexcept {
        return id;
    }

    const FileReference & file() const noexcept {
        return fileRef;
    }

    bool operator==(const SettlementPolygon<P> & other) const noexcept {
        return this->key() == other.key();
    }
};

static_assert(fishnet::geometry::IPolygon<SettlementPolygon<fishnet::geometry::Polygon<double>>>);
static_assert(DatabaseNode<SettlementPolygon<fishnet::geometry::Polygon<double>>>);