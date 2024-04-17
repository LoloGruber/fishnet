find_package(PROJ CONFIG REQUIRED)
find_package(GDAL CONFIG REQUIRED)

add_library(io_vector INTERFACE)

target_include_directories(io_vector INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(io_vector INTERFACE  geometry_objects util util_collections io_gdal_adapter PROJ::proj GDAL::GDAL)
