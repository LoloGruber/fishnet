add_library(io_gis STATIC
Shapefile.cpp
)
target_include_directories(io_gis PUBLIC ${CMAKE_CURRENT_LIST_DIR})