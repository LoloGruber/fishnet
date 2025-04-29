set(MATH_SOURCES
        src/Degrees.cpp
        src/Radians.cpp
)
add_library(math STATIC ${MATH_SOURCES})
target_include_directories(math PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)