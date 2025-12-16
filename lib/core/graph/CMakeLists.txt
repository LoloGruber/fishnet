add_subdirectory(adjacency)
add_subdirectory(model)
add_subdirectory(algorithm)
add_subdirectory(centrality)
SET(GraphLib fishnet_graph)
add_library(${GraphLib} INTERFACE)
target_link_libraries(${GraphLib} INTERFACE 
    ${FISHNET_NAME}::Graph::Adjacency
    ${FISHNET_NAME}::Graph::Algorithm
    ${FISHNET_NAME}::Graph::Model
    ${FISHNET_NAME}::Graph::Centrality
    util
)
target_include_directories(${GraphLib} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
add_library(${FISHNET_NAME}::Graph ALIAS ${GraphLib})