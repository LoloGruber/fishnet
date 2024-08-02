if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wno-unused-parameter -Wno-unused-function -fconcepts-diagnostics-depth=4)
endif()
add_subdirectory(util)
add_subdirectory(geometry)
add_subdirectory(graph)
add_subdirectory(math)
add_subdirectory(io)
set(FISHNET_TARGETS
        util
        graph
        geometry
        io
        math
)
add_library(${FISHNET_NAME} INTERFACE)
target_include_directories(${FISHNET_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(${FISHNET_NAME} INTERFACE ${FISHNET_TARGETS})
add_library(${FISHNET_NAME}::${FISHNET_NAME} ALIAS ${FISHNET_NAME} )
#export(TARGETS ${FISHNET_TARGETS} NAMESPACE fishnet:: FILE Fishnet.cmake)
#https://db.in.tum.de/teaching/ss20/c++praktikum/slides/lecture-11.pdf?lang=de