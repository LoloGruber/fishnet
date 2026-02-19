#include <fishnet/Fishnet.hpp>

using namespace fishnet;

int main() {
    using G = geometry::Polygon<double>;
    Shapefile input {"/path/to/file.shp"};
    auto inputLayer = VectorIO::read<G>(input);
    auto polygons = inputLayer.getGeometries();
    // scale aaBB of polygon by this factor; intersecting buffers -> adjacent
    double bufferMultiplier = 2; 
    size_t maximumNumberOfNeighbours = 5;
    auto adjacencies = geometry::findNeighbouringPolygons(polygons,bufferMultiplier,maximumNumberOfNeighbours);
    auto polygonGraph = graph::GraphFactory::UndirectedGraph<G>();
    polygonGraph.addEdges(adjacencies);
    // copy spatial reference from input layer
    auto resultLayer = VectorLayer<G>(inputLayer.getSpatialReference());
    auto degreeCentralityField = resultLayer.addSizeField("degCent").value_or_throw();
    for(const G & polygon: polygonGraph.getNodes()){
        Feature<G> feature {polygon};
        auto degreeCentrality = fishnet::util::size(polygonGraph.getNeighbours(polygon));
        feature.addAttribute(degreeCentralityField,degreeCentrality);
        resultLayer.addFeature(std::move(feature));
    }   
    Shapefile output = input;
    output.appendToFilename("_degree_centrality") ;
    VectorIO::overwrite(resultLayer, output);
}