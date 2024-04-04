find_package(PROJ CONFIG REQUIRED)
find_package(GDAL CONFIG REQUIRED)

add_subdirectory(gis)
add_subdirectory(gdal)

add_library(io INTERFACE
        Feature.hpp
        FieldDefinition.hpp
        ../util/functional/IDProducer.hpp
)

target_include_directories(io INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(io INTERFACE  geometry_objects util_concepts util_collections io_gdal_adapter io_gis PROJ::proj GDAL::GDAL)