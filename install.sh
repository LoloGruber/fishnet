#!/bin/sh
rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DFISHNET_TEST=OFF -DFISHNET_APPS=ON -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
cmake --build . --config Release -j 16
cmake --install .
cd ..
rm -rf build
