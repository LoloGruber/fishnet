find_package(GDAL  REQUIRED)
add_library(io_gis STATIC
    src/Shapefile.cpp
    src/Geotiff.cpp
        include/fishnet/GISFactory.hpp
        include/fishnet/GISConverter.hpp
)
target_include_directories(io_gis PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(io_gis PRIVATE util GDAL::GDAL)