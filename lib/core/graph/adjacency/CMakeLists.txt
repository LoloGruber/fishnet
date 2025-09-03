include(${FISHNET_SOURCE_DIR}/extern/json.cmake)
add_library(graph_adjacency INTERFACE)
target_include_directories(graph_adjacency INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(graph_adjacency INTERFACE util nlohmann_json::nlohmann_json)
