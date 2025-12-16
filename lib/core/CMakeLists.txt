add_subdirectory(util)
add_subdirectory(geometry)      # depends on util, math
add_subdirectory(graph)         # depends on util, geometry
add_subdirectory(math)
add_subdirectory(io)            # depends on util, geometry
add_subdirectory(clustering)    # depends on util, geometry, graph
set(FISHNET_TARGETS
        util
        graph
        geometry
        io
        math
        clustering
)
add_library(${FISHNET_NAME} INTERFACE)
target_include_directories(${FISHNET_NAME} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(${FISHNET_NAME} INTERFACE ${FISHNET_TARGETS})
add_library(${FISHNET_NAME}::${FISHNET_NAME} ALIAS ${FISHNET_NAME} )