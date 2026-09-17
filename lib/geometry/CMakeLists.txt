add_subdirectory(interfaces)
add_subdirectory(core)
add_subdirectory(ogr)
add_subdirectory(algorithm)
set(GeometryLib fishnet_geometry)
add_library(${GeometryLib} INTERFACE )
target_link_libraries(${GeometryLib} INTERFACE ${FISHNET}::Geometry::Interface ${FISHNET}::Geometry::Core ${FISHNET}::Geometry::Algorithm ${FISHNET}::Geometry::OGR)
add_library(${FISHNET}::Geometry ALIAS ${GeometryLib})

