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
template<typename R, typename F, typename G>
concept VectorLayerReader = util::UnaryFunction<R,F,util::Either<VectorLayer<G>,std::string>> && VectorGISFile<F>;

template<typename W, typename G, typename F>
concept VectorLayerWriter = util::BiFunction<W,VectorLayer<G>,F,util::Either<F,std::string>> && VectorGISFile<F>;   
} // namespace fishnet