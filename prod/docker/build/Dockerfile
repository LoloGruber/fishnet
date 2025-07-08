FROM ghcr.io/osgeo/gdal:ubuntu-small-3.9.3
# Install dependencies and build tools
RUN apt-get update && apt-get install -y \
    libssl-dev \
    build-essential \
    cmake \
    git \
    && apt-get clean
RUN git clone https://gitlab2.informatik.uni-wuerzburg.de/descartes/sos/fishnet
RUN ln -sf /usr/include /usr/include/gdal
