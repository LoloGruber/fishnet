set(filterLib workflowFilters)
add_library(${filterLib} INTERFACE 

)
target_link_libraries(${filterLib} INTERFACE Fishnet::Fishnet)
target_include_directories(${filterLib} INTERFACE ${CMAKE_CURRENT_LIST_DIR})