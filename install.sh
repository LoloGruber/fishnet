rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DFISHNET_TEST=OFF -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
cmake --build . -j 8
cmake --install .
