add_subdirectory(objects)
add_subdirectory(algorithm)
set(GeometryLib fishnet_geometry)
add_library(${GeometryLib} INTERFACE )
target_link_libraries(${GeometryLib} INTERFACE ${FISHNET}::Geometry::Objects ${FISHNET}::Geometry::Algorithm)
add_library(${FISHNET}::Geometry ALIAS ${GeometryLib})

