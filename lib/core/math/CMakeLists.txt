set(MATH_SOURCES
        src/Degrees.cpp
        src/Radians.cpp
)
set(MathLib fishnet_math)
add_library(${MathLib} STATIC ${MATH_SOURCES})
target_include_directories(${MathLib} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(${MathLib} PRIVATE ${FISHNET}::Concepts)
add_library(${FISHNET}::Math ALIAS ${MathLib})