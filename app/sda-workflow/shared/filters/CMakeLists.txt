set(filterLib filtersLib)
add_library(${filterLib} INTERFACE 

)
target_link_libraries(${filterLib} INTERFACE Fishnet::Fishnet nlohmann_json::nlohmann_json magic_enum::magic_enum)
target_include_directories(${filterLib} INTERFACE ${CMAKE_CURRENT_LIST_DIR})