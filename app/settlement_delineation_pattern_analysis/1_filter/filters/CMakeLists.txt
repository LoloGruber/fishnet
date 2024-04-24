set(filterLib workflowFilters)
add_library(${filterLib} INTERFACE 

)
target_link_libraries(${filterLib} INTERFACE Fishnet::Fishnet nlohmann_json::nlohmann_json)
target_include_directories(${filterLib} INTERFACE ${CMAKE_CURRENT_LIST_DIR})