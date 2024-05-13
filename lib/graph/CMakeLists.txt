add_subdirectory(adjacency)
add_subdirectory(model)
add_subdirectory(algorithm)
add_subdirectory(centrality)
add_library(graph INTERFACE)
target_link_libraries(graph INTERFACE 
graph_model
graph_algo
graph_centrality
graph_adjacency
)