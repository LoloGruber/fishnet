find_package(GDAL REQUIRED)

add_library(io_vector INTERFACE)

target_include_directories(io_vector INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(io_vector INTERFACE  geometry_objects util util_collections GDAL::GDAL)
