rm -rf build
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DFISHNET_TEST=OFF -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
cmake --build . --config Release -j 16
cmake --install .
cd ..
rm -rf build
