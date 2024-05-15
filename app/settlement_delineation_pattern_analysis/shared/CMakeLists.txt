add_subdirectory(settlement)
add_subdirectory(filters)
add_subdirectory(neighbourPredicates)
add_library(taskLib STATIC 
Task.hpp
)
target_include_directories(taskLib PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(taskLib PUBLIC settlementLib workflowFilters neighbourPredicateLib  util nlohmann_json::nlohmann_json memgraphAdapter)