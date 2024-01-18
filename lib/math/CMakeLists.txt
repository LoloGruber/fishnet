add_library(math_concepts INTERFACE)
target_include_directories(math_concepts INTERFACE ${CMAKE_CURRENT_LIST_DIR}/concepts)

set(MATH_SOURCES
Degrees.cpp
Radians.cpp
)

add_library(math STATIC ${MATH_SOURCES})
target_include_directories(math PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(math PUBLIC math_concepts)