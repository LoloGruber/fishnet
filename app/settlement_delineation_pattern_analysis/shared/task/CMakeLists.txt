add_library(taskLib STATIC 
Task.hpp
TaskConfig.hpp
)
target_include_directories(taskLib PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(taskLib PUBLIC nlohmann_json::nlohmann_json util)