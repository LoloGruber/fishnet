find_package(GDAL REQUIRED)

include_directories(include ${GDAL_INCLUDE_DIRS})

add_subdirectory(gis)
add_subdirectory(gdalAdapter)

add_library(io INTERFACE
# VectorDataset.h 
# VectorDataset.cpp
# VectorLayer.hpp
# converters/VectorDatasetConverter.h
# converters/GeotiffToVectorDataset.h
# converters/GeotiffToVectorDataset.cpp
# gis-file/type/WSFTypeEnum.h
# gis-file/type/WSFType.h
# gis-file/type/WSFType.cpp
# gis-file/GISFile.h
# gis-file/GISFile.cpp
# gis-file/GeoTIFF.h
# gis-file/GeoTIFF.cpp
# gis-file/Shapefile.h
# gis-file/Shapefile.cpp
)
target_include_directories(io INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(io INTERFACE  geometry_objects util_concepts util_collections io_gdal_adapter io_gis ${GDAL_LIBRARIES})