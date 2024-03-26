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
#export(TARGETS ${FISHNET_TARGETS} NAMESPACE fishnet:: FILE Fishnet.cmake)
#https://stackoverflow.com/questions/47060164/cmake-linking-shared-library-no-such-file-or-directory-when-include-a-header
#https://db.in.tum.de/teaching/ss20/c++praktikum/slides/lecture-11.pdf?lang=de