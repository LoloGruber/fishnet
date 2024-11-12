FROM ubuntu:24.04
# Install dependencies and build tools
RUN apt-get update && apt-get install -y \
    libgdal-dev \
    build-essential \
    cmake \
    git \
    && apt-get clean
# Clone Repository
RUN git clone https://gitlab2.informatik.uni-wuerzburg.de/log66jr/fishnet
# Build the project
RUN mkdir /fishnet/build
WORKDIR /fishnet/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DFISHNET_TEST=OFF -DFISHNET_APPS=ON -DCMAKE_INSTALL_LIBDIR=lib ..
RUN cmake --build . --config Release -j 16
RUN cmake --install .