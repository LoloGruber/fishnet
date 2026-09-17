add_library(fishnet_geometry_interface INTERFACE)
target_include_directories(fishnet_geometry_interface INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(fishnet_geometry_interface INTERFACE ${FISHNET}::Concepts ${FISHNET}::Math ${FISHNET}::Functional)
add_library(${FISHNET}::Geometry::Interface ALIAS fishnet_geometry_interface)
