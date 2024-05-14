#pragma once

//TODO

/**
 * @brief Wrapper Object, which fulfills the requirements of a Multi-Polygon and for a Node stored in the Memgraph DB
 * 
 * @tparam P specific polygon type (e.g. Polygon<double>)
 */
template<fishnet::geometry::IMultiPolygon P>
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