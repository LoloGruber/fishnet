FROM logru/fishnet-deps:latest AS build
RUN git clone https://gitlab2.informatik.uni-wuerzburg.de/descartes/sos/fishnet
RUN mkdir -p /fishnet/build
WORKDIR /fishnet/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DFISHNET_TEST=OFF -DFISHNET_TEST_LIB=OFF -DFISHNET_APPS=ON .. 
RUN cmake --build . --config Release -j 8
RUN cmake --install .

# Runtime stage
FROM ghcr.io/osgeo/gdal:ubuntu-small-3.9.3 AS runtime
COPY --from=build /usr/local/bin /usr/local/bin
COPY --from=build /usr/local/lib /usr/local/lib
