find_package(GDAL REQUIRED)
set(IOVectorLib fishnet_io_vector)
add_library(${IOVectorLib} INTERFACE)

target_include_directories(${IOVectorLib} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(${IOVectorLib} INTERFACE  ${FISHNET}::Geometry::Objects ${FISHNET}::Concepts GDAL::GDAL)
add_library(${FISHNET}::IO::Vector ALIAS ${IOVectorLib})