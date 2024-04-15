add_subdirectory(collections)
add_subdirectory(console)
add_subdirectory(hashing)
add_subdirectory(functional)
add_subdirectory(filesystem)

add_library(util INTERFACE ${UTIL_SOURCES})
target_link_libraries(util INTERFACE util_hashing util_collections util_filesystem util_functional util_console)
target_include_directories(util INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
