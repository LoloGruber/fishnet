#pragma once
#include <fishnet/VectorLayer.hpp>
#include <fishnet/Shapefile.hpp>

namespace fishnet {

class ShapefileIO {
public:
    template<typename G>
    VectorLayer<G> operator(const Shapefile & shapefile) const {
        
    }

};

} // namespace fishnet