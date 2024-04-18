add_library(io_gdal_adapter INTERFACE
        include/fishnet/WGS84Ellipsoid.hpp
)

find_package(GDAL  REQUIRED)

target_include_directories(io_gdal_adapter INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(io_gdal_adapter INTERFACE geometry_objects GDAL::GDAL)

