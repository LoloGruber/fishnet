add_library(jobLib INTERFACE)
target_include_directories(jobLib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(jobLib INTERFACE memgraphAdapter graph_adjacency magic_enum::magic_enum nlohmann_json::nlohmann_json)