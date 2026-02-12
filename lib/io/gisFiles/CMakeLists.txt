find_package(GDAL  REQUIRED)
set(GISLib fishnet_io_gis)
add_library(${GISLib} STATIC
    src/Shapefile.cpp
    src/Geotiff.cpp
    src/GeoPackage.cpp
        include/fishnet/GISFactory.hpp
        include/fishnet/GISConverter.hpp
)
target_include_directories(${GISLib} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(${GISLib} PRIVATE ${FISHNET}::Concepts ${FISHNET}::Functional ${FISHNET}::IO::Filesystem GDAL::GDAL)
add_library(${FISHNET}::IO::GIS ALIAS ${GISLib})