add_library(graph_model INTERFACE)
target_link_libraries(graph_model INTERFACE util graph_adjacency)
target_include_directories(graph_model INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
