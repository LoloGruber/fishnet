FROM fishnet/cwl:latest
# Clone Repository
RUN git clone https://gitlab2.informatik.uni-wuerzburg.de/log66jr/fishnet
# Build the project
RUN mkdir /fishnet/build
WORKDIR /fishnet/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DFISHNET_TEST=OFF -DFISHNET_APPS=ON -DCMAKE_INSTALL_LIBDIR=lib ..
RUN cmake --build . --config Release -j 16
RUN cmake --install .
WORKDIR /fishnet
RUN rm -rf build/
WORKDIR /data
CMD ["tail", "-f", "/dev/null"]
