add_library(taskLib STATIC 
Task.hpp
JsonConfigReader.hpp
)
target_include_directories(taskLib PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(taskLib PRIVATE util nlohmann_json::nlohmann_json)