add_library(graph INTERFACE)
target_link_libraries(graph INTERFACE 
graph_model
graph_algo
graph_centrality
graph_adjacency
)
target_include_directories(graph INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)