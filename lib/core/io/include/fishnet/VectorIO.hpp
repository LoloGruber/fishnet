#pragma once
#include <fishnet/VectorLayer.hpp>
#include <fishnet/ShapefileIO.hpp>
#include <fishnet/Either.hpp>
#include <regex>

namespace fishnet::__impl{
/**
 * @brief Wrapper for generic VectorLayerWriter that does not overwrite existing files by changing the filename with a mapper function
 * 
 * @tparam G Geometry type of Layer
 * @tparam F VectorGISFile type
 * @tparam VectorLayerWriterType 
 */
template<typename G,VectorGISFile F, VectorLayerWriter<G,F> VectorLayerWriterType>
class NonOverwritingVectorLayerWriter {
private:
    VectorLayerWriterType writer;
    fishnet::util::UnaryFunction_t<F,F> filenameMapper;
public:
    explicit NonOverwritingVectorLayerWriter(VectorLayerWriterType const& writer, fishnet::util::UnaryFunction<F,F> auto && filenameMapper) 
    : writer(std::move(writer)),filenameMapper(std::move(filenameMapper)) {}

    util::Either<F,std::string> operator()(const VectorLayer<G> & layer, const F & destination) const {
        F outputFile = destination;
        if(destination.exists()){
            outputFile = filenameMapper(destination);
        }
        return writer(layer, outputFile);
    }
};

template<GISFile F>
struct IncrementFilenameMapper {
    F operator()(const F & file) const {
        F output = file;
        std::regex endsWithVersionRegex {"_(\\d+)$"};
        const std::string & currentFilename = file.getPath().stem().string();
        std::smatch matcher;
        if(std::regex_search(currentFilename,matcher,endsWithVersionRegex)){
            size_t splitIndex = currentFilename.find_last_of('_');
            std::string filename = currentFilename.substr(0,splitIndex);
            int version = std::stoi(matcher[1].str()) + 1;
            output.changeFilename(filename+"_"+std::to_string(version));
        }else {
            output.appendToFilename("_1");
        }
        return output;
    }
};
} // namespace fishnet::__impl

namespace fishnet::VectorIO {

/**
 * @brief Creates an empty VectorLayer with the given spatial reference
 * 
 * @tparam G geometry type of the layer
 * @param spatialReference spatial reference for the layer
 * @return VectorLayer<G> empty vector layer with the given spatial reference
 */
template<geometry::GeometryObject G>
VectorLayer<G> empty(const OGRSpatialReference & spatialReference) {
    return VectorLayer<G>(spatialReference);
}

/**
 * @brief Creates an empty VectorLayer with the same spatial reference and fields as the source layer
 * 
 * @tparam G geometry type of the layer
 * @param source source layer to copy the spatial reference and fields from
 * @return VectorLayer<G> empty vector layer with the same spatial reference and fields as the source layer
 */
template<geometry::GeometryObject G>
VectorLayer<G> emptyCopy(const VectorLayer<G> & source) {
    auto layer = empty<G>(source.getSpatialReference());
    source.copyFields(layer);
    return layer;
}

template<VectorLayerReader Reader>
auto tryRead(const Reader & reader, const typename Reader::file_type & file) -> std::remove_cvref_t<decltype(reader(file))> {
    return reader(file);
}

template<VectorLayerReader Reader>
auto read(const Reader & reader, const typename Reader::file_type & file) -> std::remove_cvref_t<decltype(tryRead(reader,file).value())> {
    using F = typename Reader::file_type;
    using G = typename Reader::geometry_type;
    static_assert(VectorLayerReader<Reader,F,G>, "Reader does not satisfy VectorLayerReader concept");
    return tryRead(reader,file).value_or_throw();
}

template<geometry::GeometryObject G>
VectorLayer<G> read(const Shapefile & shapefile) {
    return read(ShapefileReader<G>(), shapefile);
} 

inline VectorLayer<geometry::Polygon<double>> readPolygonLayer(const Shapefile & shapefile) {
    return read<geometry::Polygon<double>>(shapefile);
}

template<geometry::GeometryObject G,VectorGISFile F>
util::Either<F,std::string> tryOverwrite(const VectorLayerWriter<G,F> auto & writer, const VectorLayer<G> & layer, const F & destination){
    return writer(layer, destination);
}

template<geometry::GeometryObject G,VectorGISFile F>
util::Either<F,std::string> tryWrite(const VectorLayerWriter<G,F> auto & writer, const VectorLayer<G> & layer, const F & destination){
    auto nonOverwritingWriter = __impl::NonOverwritingVectorLayerWriter<G,F,std::remove_cvref_t<decltype(writer)>>(writer,__impl::IncrementFilenameMapper<F>());
    return tryOverwrite(nonOverwritingWriter, layer, destination);
}

template<geometry::GeometryObject G,VectorGISFile F>
F write(const VectorLayerWriter<G,F> auto & writer, const VectorLayer<G> & layer, const F & destination){
    return tryWrite(writer, layer, destination).value_or_throw();
}

template<geometry::GeometryObject G,VectorGISFile F>
F overwrite(const VectorLayerWriter<G,F> auto & writer, const VectorLayer<G> & layer, const F & destination) {
    return tryOverwrite(writer, layer, destination).value_or_throw();
}

template<geometry::GeometryObject G>
Shapefile write(const VectorLayer<G> & layer, const Shapefile & destination) {
    return write(ShapefileWriter<G>(), layer, destination);
}

template<geometry::GeometryObject G>
Shapefile overwrite(const VectorLayer<G> & layer, const Shapefile & destination) {
    return overwrite(ShapefileWriter<G>(), layer, destination);
}

} // namespace fishnet::VectorIO