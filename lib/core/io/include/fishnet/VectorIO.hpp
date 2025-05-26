#pragma once
#include <fishnet/VectorLayer.hpp>
#include <fishnet/ShapefileIO.hpp>
#include <fishnet/Either.hpp>

namespace fishnet::VectorIO {

template<geometry::GeometryObject G,VectorGISFile F>
VectorLayer<G> read(const VectorLayerReader<F,VectorLayer<G>> auto & reader, const F & file) {
    return reader(file);
}

template<geometry::GeometryObject G>
VectorLayer<G> read(const Shapefile & shapefile) {
    ShapefileReader<G> shapefileIO;
    return shapefileIO(shapefile).value_or_throw();
} 

}// namespace fishnet