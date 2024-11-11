cd ../..
rm -rf build
mkdir build
cd build
cmake -DFISHNET_COVERAGE=ON -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
make -j 16
ctest
lcov --gcov-tool /usr/bin/gcov-13 --capture --directory . --output-file ../doc/coverage/coverage.info
cd ../tests/coverage
lcov --remove coverage.info '/usr/*' --output-file coverage.info
lcov --remove coverage.info '*googletest-src*' --output-file coverage.info
lcov --remove coverage.info '*.local*' --output-file coverage.info
lcov --remove coverage.info '*extern/*' --output-file coverage.info
genhtml coverage.info --output-directory report
./DisplayCoverage.sh
