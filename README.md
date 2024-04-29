# Fishnet:
A framework for graph-based computations and analysis of GIS data.

### Installation:
Before the library can be build using *cmake*, the **GDAL** and **SSL** libraries have to be installed on the machine. On Ubuntu-based system this can be achieved using the following command:
```
sudo apt install -y libgdal-dev libssl-dev
``` 
Additionally, the required external submodules need to be initialized using:
```
git submodule init
git submodule update
```
Then the project can be build as follows:
```
mkdir build
cd build
cmake ..
cmake --build . <add custom cmake parameters here>
```
When utilizing the **Memgraph** client, a running instance of the **Memgraph** database can be obtained using docker compose:
```
cd prod/memgraph
sudo docker compose up -d
```



