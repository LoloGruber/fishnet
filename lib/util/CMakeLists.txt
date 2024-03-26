add_subdirectory(collections)
add_subdirectory(console)
add_subdirectory(hashing)
add_subdirectory(functional)
add_subdirectory(filesystem)

add_library(util_concepts INTERFACE)
target_link_libraries(util_concepts INTERFACE util_collections_concepts util_functional_concepts util_hashing_concepts math_concepts util_console_concepts)
target_include_directories(util_concepts INTERFACE ${CMAKE_CURRENT_LIST_DIR})

add_library(util INTERFACE ${UTIL_SOURCES})
target_link_libraries(util INTERFACE util_concepts util_hashing util_collections util_filesystem)
target_include_directories(util INTERFACE ${CMAKE_CURRENT_LIST_DIR}/inc)
