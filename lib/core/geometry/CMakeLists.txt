add_subdirectory(objects)
add_subdirectory(algorithm)
set(GeometryLib fishnet_geometry)
add_library(${GeometryLib} INTERFACE )
target_link_libraries(${GeometryLib} INTERFACE geometry_objects geometry_algo)
add_library(${FISHNET}::Geometry ALIAS ${GeometryLib})

