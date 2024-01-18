add_executable(ioTest
VectorLayerTest.cpp
OGRAdapterTest.cpp
ShapefileTest.cpp
)
#find_package(GDAL REQUIRED)
gtest_discover_tests(ioTest)
# include_directories(include ${GDAL_INCLUDE_DIRS})
target_link_libraries(ioTest io testutil util_filesystem
#${GDAL_LIBRARIES}
)