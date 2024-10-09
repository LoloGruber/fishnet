FROM gcc:13
# Install dependencies and build tools
RUN apt-get update && apt-get install -y \
    libgdal-dev \
    libssl-dev \
    build-essential \
    cmake \
    git \
    nodejs \
    npm \
    python3 \
    python3-pip \
    python3-venv && \
    apt-get clean
# Create venv for installation of toil
RUN python3 -m venv /opt/venv 
# Install toil
RUN /opt/venv/bin/pip install toil[cwl]
# Add to PATH
RUN export PATH="/opt/venv/bin:$PATH"
# Clone Repository
RUN git clone https://gitlab2.informatik.uni-wuerzburg.de/log66jr/fishnet
# Copy cwl and cfg files in user directory
WORKDIR /fishnet
RUN cp -r prod/cfg /root/cfg
# Build the project
RUN mkdir build
WORKDIR /fishnet/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DFISHNET_TEST=OFF -DFISHNET_APPS=ON -DCMAKE_INSTALL_LIBDIR=lib ..
RUN cmake --build . --config Release -j 16
RUN cmake --install .
# Initialize home directory
WORKDIR /root
RUN mkdir jobs

CMD ["tail", "-f", "/dev/null"]