# Fishnet
A framework for graph-based computations and analysis of GIS data.

## Installation
### System Installation
Before the library can be build using *cmake*, the **GDAL** and **SSL** libraries have to be installed on the machine. On Ubuntu-based system this can be achieved using the following command:
```shell
sudo apt-get install -y libgdal-dev libssl-dev
``` 
Then the project can be build as follows:
```shell
mkdir build
cd build
cmake ..
cmake --build . <add custom cmake parameters here>
```
## SDA Workflow
To run the Settlement Delineation and Analysis workflow, the easiest way is to use the docker-compose. 
### Docker
- The [logru/fishnet:base](/prod/docker/base/Dockerfile) image contains the required dependencies to build the project. 
- To execute the **sda workflow** the [logru/fishnet:sda](app/sda-workflow/Dockerfile) image builds the required binaries. 

Since the workflow requires a running memgraph instance, the [docker-compose](app/sda-workflow/compose.yaml) file joins both the memgraph database and the **fishnet/sda** image.
```shell
cd app/sda-workflow
docker compose up -d
```
Then change your directory to the mounted directory, specified in the [docker-compose](app/sda-workflow/compose.yaml), by default: **~/fishnet**. The workflow can then be executed using the *docker exec*, specifying the inputs/config/outputs as relative paths from the mounted directory.
```docker
docker exec fishnet SettlementDelineation -i <input> -c <cfg> -o <output>

For example:

docker exec fishnet SettlementDelineation -i input/Astana_KAZ.tiff -c cfg/sda-workflow.json -o output/AstanaAnalysis.shp
```

### System
Alternatively the sda workflow can be installed on the system using the [install](install.sh) script. Make sure that the install prefix location is referenced in *PATH* (e.g. *usr/local/bin*). 
```shell
./install.sh
```
Additionally, a cwl runner must be installed to execute the individual stages of the workflow. The reference executor [cwltool](https://cwltool.readthedocs.io/en/latest/cli.html#cwltool) is recommended and can be installed as follows:
```shell
sudo apt-get install cwltool
```
Furthermore, a running instance of the **Memgraph** database joined with the **Memgraph Lab** web interface can be obtained using docker compose:
```shell
cd prod/local
docker compose up -d
```
The workflow can then be executed as follows:
```shell
SettlementDelineation -i <path-to-input> -c <path-to-cfg> -o <path-to-output.shp>
```
## Framework Usage
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


