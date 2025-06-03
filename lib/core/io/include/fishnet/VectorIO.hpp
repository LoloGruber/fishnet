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

template<VectorGISFile F,geometry::GeometryObject G>
util::Either<VectorLayer<G>,std::string> tryRead(const VectorLayerReader<F,G> auto & reader, const F & file) {
    return reader(file);
}

template<VectorGISFile F,geometry::GeometryObject G>
VectorLayer<G> read(const VectorLayerReader<F,G> auto & reader, const F & file) {
    return tryRead<F,G>(reader,file).value_or_throw();
}

template<geometry::GeometryObject G>
VectorLayer<G> read(const Shapefile & shapefile) {
    return read<Shapefile,G>(ShapefileReader<G>(), shapefile);
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