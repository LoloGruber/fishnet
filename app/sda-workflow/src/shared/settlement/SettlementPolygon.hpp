#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/MemgraphAdjacency.hpp>

/**
 * @brief Settlement Object, which fulfills the requirements of a Polygon and for a Node stored in the Memgraph DB.
 * 
 * @tparam S specific shape type (e.g. Polygon<double>)
 */
template<fishnet::geometry::Shape S>
class SettlementPolygon:public S{
private:
    size_t id; // unique id of the settlement
    FileReference fileRef; // file, which stores the shape
public:
    template<typename... Args>
    SettlementPolygon(size_t id, FileReference fileRef, Args&&... args):S(std::forward<Args>(args)...),id(id),fileRef(std::move(fileRef)){}
    size_t key() const noexcept {
        return id;
    }

    const FileReference & file() const noexcept {
        return fileRef;
    }

    bool operator==(const SettlementPolygon<S> & other) const noexcept {
        return this->key() == other.key();
    }
};