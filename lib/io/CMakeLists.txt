find_package(GDAL REQUIRED)

include_directories(include ${GDAL_INCLUDE_DIRS})

add_subdirectory(gis)
add_subdirectory(gdalAdapter)

add_library(io INTERFACE)

target_include_directories(io INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(io INTERFACE  geometry_objects util_concepts util_collections io_gdal_adapter io_gis ${GDAL_LIBRARIES})