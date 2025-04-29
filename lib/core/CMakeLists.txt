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