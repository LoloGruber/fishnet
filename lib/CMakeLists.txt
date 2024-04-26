if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wno-unused-parameter -Wno-unused-function -fconcepts-diagnostics-depth=4)
endif()
set(LIBRARY_NAME Fishnet)
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
add_library(${LIBRARY_NAME} INTERFACE)
target_link_libraries(${LIBRARY_NAME} INTERFACE ${FISHNET_TARGETS})
add_library(${LIBRARY_NAME}::${LIBRARY_NAME} ALIAS ${LIBRARY_NAME} )
#export(TARGETS ${FISHNET_TARGETS} NAMESPACE fishnet:: FILE Fishnet.cmake)
#https://stackoverflow.com/questions/47060164/cmake-linking-shared-library-no-such-file-or-directory-when-include-a-header
#https://db.in.tum.de/teaching/ss20/c++praktikum/slides/lecture-11.pdf?lang=de