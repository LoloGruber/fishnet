#install ubuntu
FROM ubuntu:20.04 AS dependencies

ENV name="wsf_network"

ENV DEBIAN_FRONTEND noninteractive

#install required dynamic link libraries
RUN  set -ex;    \
    apt-get update && apt-get install -y libgdal-dev libcgal-dev;\
    apt-get clean;

FROM dependencies AS build
# install compiler and cmake for the build environment
RUN apt-get install -y build-essential cmake; mkdir -p /wsf;
#copy source code into container
COPY . ./wsf

WORKDIR /wsf
#compile program using cmake
RUN cd /wsf; cmake -DCMAKE_BUILD_TYPE=Release -G 'CodeBlocks - Unix Makefiles' -S . -B build;cd build ; cmake --build . --target $name -- -j 9;

# create runtime environment, requiring the libraries specified in the base container
FROM dependencies as runtime

RUN mkdir wsf;cd wsf; mkdir data
# copy compiled programm from build into runtime environment
COPY --from=build /wsf/build/$name /wsf/

WORKDIR /wsf
#keep container alive, wait for user to connect and run the program with a dataset
CMD ["sleep", "infinity"]






# libs:
#   libcdal-dev
#   libboost-dev
#   libgdal-dev
#
#
# Cmake Build:
#$ cd '/mnt/c/Users/grube/OneDrive/Dokumente/Informatik Bachelor/6_SS21/Bachelorarbeit/Code/cmake-build-debug' && /usr/bin/cmake --build '/mnt/c/Users/grube/OneDrive/Dokumente/Informatik Bachelor/6_SS21/Bachelorarbeit/Code/cmake-build-debug' --target wsf_network -- -j 9"
