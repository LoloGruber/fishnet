#pragma once

#include <fishnet/VectorLayer.hpp>
#include <fishnet/Either.hpp>

namespace fishnet {

 /**
 * @brief Interface for reading a VectorLayer from a file
 * 
 * @tparam R reader type
 * @tparam F gis file type
 * @tparam G geometry type of layer
 */
template<typename R, typename F=R::file_type, typename G=R::geometry_type>
concept VectorLayerReader = util::UnaryFunction<R,F,util::Either<VectorLayer<G>,std::string>> && VectorGISFile<F> && requires(){
    typename R::geometry_type;
    typename R::file_type;
};

template<typename W, typename G, typename F>
concept VectorLayerWriter = util::BiFunction<W,VectorLayer<G>,F,util::Either<F,std::string>> && VectorGISFile<F>;   
} // namespace fishnet