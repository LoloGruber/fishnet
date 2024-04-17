
# find_package(GDAL REQUIRED)
# include_directories(include ${GDAL_INCLUDE_DIRS})
add_subdirectory(objects)
add_subdirectory(algorithm)

set(GEOMETRY_SOURCES 
    # src/GeometryAlgorithm.cpp
    # src/Line.cpp
    # src/Polygon.cpp
    # src/Ring.cpp
    # # src/Vec2D.cpp
    # src/CharacteristicShape.cpp
    # src/CombinatorialMap.cpp
    # src/Dart.cpp
    # src/DartEdge.cpp
    # src/Vertex.cpp
)

add_library(geometry INTERFACE # ${GEOMETRY_SOURCES}
)
target_link_libraries(geometry INTERFACE 
geometry_objects 
#PRIVATE util ${GDAL_LIBRARIES} 
)
# target_include_directories(geometry PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
