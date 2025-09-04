add_library(geometryTestUtils OBJECT 
ShapeSamples.cpp
)

target_link_libraries(geometryTestUtils PRIVATE geometry)
target_include_directories(geometryTestUtils PUBLIC ${CMAKE_CURRENT_LIST_DIR})