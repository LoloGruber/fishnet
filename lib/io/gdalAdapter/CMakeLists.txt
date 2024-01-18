add_library(io_gdal_adapter INTERFACE)

find_package(GDAL REQUIRED)

include_directories(include ${GDAL_INCLUDE_DIRS})

target_include_directories(io_gdal_adapter INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(io_gdal_adapter INTERFACE geometry_objects ${GDAL_LIBRARIES})

