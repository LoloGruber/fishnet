#!/bin/sh
BUILD_DIRECTORY="build-release"
# rm -rf $BUILD_DIRECTORY
mkdir $BUILD_DIRECTORY
cd $BUILD_DIRECTORY
cmake -DCMAKE_BUILD_TYPE=Release -DFISHNET_TEST=OFF -DFISHNET_APPS=ON -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
cmake --build . --config Release -j 16
cmake --install .
cd ..
# rm -rf $BUILD_DIRECTORY
