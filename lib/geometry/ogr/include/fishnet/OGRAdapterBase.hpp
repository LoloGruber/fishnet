#pragma once
#include <gdal/ogr_geometry.h>
#include <memory>
#include <cassert>

namespace fishnet::geometry{
struct OGRGeometryDeleter {
    void operator()(OGRGeometry* geom) const {
        if(geom)
            OGRGeometryFactory::destroyGeometry(geom);
    }
};

template<typename T>
using OGRUniquePtr = std::unique_ptr<T, OGRGeometryDeleter>;

template<typename T> requires std::is_base_of_v<OGRGeometry, T>
class OGRAdapterBase{
protected:
    OGRUniquePtr<T> geomPtr;
public:
    using numeric_type = double;

    OGRAdapterBase(OGRUniquePtr<T> && geomPtr):geomPtr(std::move(geomPtr)){
        assert(this->geomPtr != nullptr);
    }

    OGRAdapterBase(OGRAdapterBase && other) noexcept : geomPtr(std::move(other.geomPtr)) {}

    OGRAdapterBase(const OGRAdapterBase & other): geomPtr(OGRUniquePtr<T>(static_cast<T*>(other.geomPtr->clone()))) {}

    OGRAdapterBase& operator=(const OGRAdapterBase & other){
        if(this != &other){
            geomPtr = OGRUniquePtr<T>(static_cast<T*>(other.geomPtr->clone()));
        }
        return *this;
    }
    OGRAdapterBase& operator=(OGRAdapterBase && other) noexcept {
        if(this != &other){
            geomPtr = std::move(other.geomPtr);
        }
        return *this;
    }

    bool operator==(const OGRAdapterBase & other) const noexcept{
        if(this->geomPtr == other.geomPtr) return true; // same pointer, same object
        return geomPtr->Equals(other.geomPtr.get());
    }

    T * raw() const noexcept{
        return geomPtr.get();
    }

    size_t hash() const noexcept {
        size_t nSize = geomPtr->WkbSize();
        std::vector<unsigned char> pabyData(nSize);
        geomPtr->exportToWkb(wkbNDR, pabyData.data());
        std::string_view bytes(reinterpret_cast<const char*>(pabyData.data()), nSize);
        return std::hash<std::string_view>{}(bytes);
    }

    std::string toString() const{
        return geomPtr->exportToWkt();
    }
};
}
