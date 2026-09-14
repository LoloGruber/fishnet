set(GDALAdapterLib fishnet_io_gdal_adapter)
add_library(${GDALAdapterLib} INTERFACE)
target_include_directories(${GDALAdapterLib} INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(${GDALAdapterLib} INTERFACE ${FISHNET}::Geometry::Core GDAL::GDAL spdlog::spdlog)
add_library(${FISHNET}::IO::GDAL ALIAS ${GDALAdapterLib})

