add_library(geometryTestUtils STATIC 
ShapeSamples.cpp
)

target_link_libraries(geometryTestUtils PRIVATE geometry)
target_include_directories(geometryTestUtils PUBLIC ${CMAKE_CURRENT_LIST_DIR})