add_subdirectory(concepts)
add_subdirectory(clustering)
add_subdirectory(collections)
add_subdirectory(functional)
add_subdirectory(geometry)
add_subdirectory(graph)
add_subdirectory(io)
add_subdirectory(math)
add_subdirectory(workflow)
set(FishnetLibs
        ${FISHNET}::Concepts
        ${FISHNET}::Clustering
        ${FISHNET}::Collections
        ${FISHNET}::Functional
        ${FISHNET}::Geometry
        ${FISHNET}::Graph
        ${FISHNET}::IO
        ${FISHNET}::Math
)

add_library(${FISHNET} INTERFACE)
target_include_directories(${FISHNET} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(${FISHNET} INTERFACE ${FishnetLibs})
add_library(${FISHNET}::${FISHNET} ALIAS ${FISHNET} )