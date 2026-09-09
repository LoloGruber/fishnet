add_library(geometry_interfaces INTERFACE)
target_include_directories(geometry_interfaces INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(geometry_interfaces INTERFACE ${FISHNET}::Concepts ${FISHNET}::Math ${FISHNET}::Functional)
add_library(${FISHNET}::Geometry::Interface ALIAS geometry_interfaces)
