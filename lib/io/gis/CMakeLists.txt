add_library(io_gis STATIC
    src/Shapefile.cpp
)
target_include_directories(io_gis PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
