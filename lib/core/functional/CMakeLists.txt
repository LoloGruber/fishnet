set(FunctionalLib fishnet_functional)
add_library(${FunctionalLib} INTERFACE)
target_include_directories(${FunctionalLib} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
add_library(${FISHNET_NAME}::Functional ALIAS ${FunctionalLib})