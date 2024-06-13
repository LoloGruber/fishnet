FROM gcc:13
ENV homeDir="/home/fishnet"
RUN apt-get update && apt-get install -y libgdal-dev libssl-dev build-essential cmake git;\
    apt-get clean;
RUN git clone https://gitlab2.informatik.uni-wuerzburg.de/log66jr/fishnet
WORKDIR /fishnet
RUN mkdir ${homeDir}
RUN cp -r app/settlement_delineation_pattern_analysis/cwl/ ${homeDir}/
RUN cp -r app/settlement_delineation_pattern_analysis/cfg/ ${homeDir}/
RUN chmod +x install.sh
RUN git submodule init
RUN git submodule update
RUN mkdir build
WORKDIR /fishnet/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DFISHNET_TEST=OFF -DFISHNET_APPS=ON -DCMAKE_INSTALL_LIBDIR=lib ..
RUN cmake --build . --config Release -j 16
RUN cmake --install .
WORKDIR ${homeDir}
RUN mkdir jobs
RUN mkdir workingDirectory
CMD ["tail", "-f", "/dev/null"]