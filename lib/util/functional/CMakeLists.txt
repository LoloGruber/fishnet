add_library(util_functional_concepts INTERFACE)
target_include_directories(util_functional_concepts INTERFACE ${CMAKE_CURRENT_LIST_DIR}/concepts)

add_library(util_functional INTERFACE
        CompositePredicate.hpp)
target_include_directories(util_functional INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(util_functional INTERFACE util_functional_concepts)