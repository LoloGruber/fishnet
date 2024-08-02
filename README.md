# Fishnet:
A framework for graph-based computations and analysis of GIS data.

### Installation:
Before the library can be build using *cmake*, the **GDAL** and **SSL** libraries have to be installed on the machine. On Ubuntu-based system this can be achieved using the following command:
```shell
sudo apt install -y libgdal-dev libssl-dev
``` 
Additionally, the required external submodules need to be initialized using:
```shell
git submodule init
git submodule update
```
Then the project can be build as follows:
```shell
mkdir build
cd build
cmake ..
cmake --build . <add custom cmake parameters here>
```
When utilizing the **Memgraph** client, a running instance of the **Memgraph** database can be obtained using docker compose:
```shell
cd prod/memgraph
sudo docker compose up -d
```
### Usage:
The following example shows how to store polygons, obtained from a Shapefile, in a graph. Thereafter, the degree centrality measures is calculated on the graph and the results stored as features in the output shapefile.
```cpp
#include <fishnet/Fishnet.hpp>

using namespace fishnet;

int main() {
    using G = geometry::Polygon<double>;
    Shapefile input {"/path/to/file.shp"};
    auto inputLayer = VectorLayer<G>::read(input);
    auto polygons = inputLayer.getGeometries();
    // scale aaBB of polygon by this factor; intersecting buffers -> adjacent
    double bufferMultiplier = 2; 
    size_t maximumNumberOfNeighbours = 5;
    auto adjacencies = geometry::findNeighbouringPolygons(polygons,bufferMultiplier,maximumNumberOfNeighbours);
    auto polygonGraph = graph::GraphFactory::UndirectedGraph<G>();
    polygonGraph.addEdges(adjacencies);
    // copy spatial reference from input layer
    auto resultLayer = VectorLayer<G>::empty(inputLayer.getSpatialReference());
    auto degreeCentralityFieldExpected = resultLayer.addSizeField("degCent");
    if(not degreeCentralityFieldExpected){
        return 1; // Could not create field on layer
    }
    auto degreeCentralityField = degreeCentralityFieldExpected.value();
    for(const G & polygon: polygonGraph.getNodes()){
        Feature<G> feature {polygon};
        auto degreeCentrality = fishnet::util::size(polygonGraph.getNeighbours(polygon));
        feature.addAttribute(degreeCentralityField,degreeCentrality);
        resultLayer.addFeature(std::move(feature)); // store feature in layer
    }   
    Shapefile output = input.appendToFilename("_degree_centrality") ;
    resultLayer.overwrite(output);
}
```
To link the *Fishnet* framework to the program the following *CMake* file can be used:
```cmake 
add_executable(polygonGraph PolygonGraph.cpp)
target_link_libraries(polygonGraph PRIVATE Fishnet::Fishnet)
```


