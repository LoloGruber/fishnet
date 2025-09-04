FROM logru/fishnet-build:latest AS build
RUN mkdir -p /fishnet/build
WORKDIR /fishnet/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DFISHNET_TEST=OFF -DFISHNET_APPS=ON .. 
RUN cmake --build . --config Release -j 8
RUN cmake --install .

# Runtime stage
FROM ghcr.io/osgeo/gdal:ubuntu-small-3.9.3 AS runtime
RUN apt-get update && apt-get install -y cwltool  && apt-get clean
COPY --from=build /usr/local/bin /usr/local/bin
COPY --from=build /usr/local/lib /usr/local/lib
COPY --from=build /fishnet/cwl/sda /cwl
RUN mkdir -p /data
RUN mkdir -p /working
WORKDIR /data/working

# Setup Memgraph
FROM runtime AS deployment
RUN apt-get update && apt-get install -y wget adduser
WORKDIR /tmp
RUN wget -O memgraph.deb https://download.memgraph.com/memgraph/v3.1.1/ubuntu-24.04/memgraph_3.1.1-1_amd64.deb
RUN dpkg -i memgraph.deb
RUN rm memgraph.deb
WORKDIR /data/working
CMD [ "su", "memgraph" , "-c", "/usr/lib/memgraph/memgraph --query-execution-timeout-sec=0 --storage-mode=IN_MEMORY_ANALYTICAL" ]