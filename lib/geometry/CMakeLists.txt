add_subdirectory(objects)
add_subdirectory(algorithm)

add_library(geometry INTERFACE )
target_link_libraries(geometry INTERFACE 
geometry_objects 
geometry_algo
)

