add_subdirectory(settlement_patterns)
add_subdirectory(settlement_filter)


#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../lib)
#add_executable(wsf_network main.cpp WSFNetworkApplication.cpp WSFNetworkApplication.h)

#add_dependencies(wsf_network geometry graph io util)
# find_package(GDAL REQUIRED)
# include_directories(include ${GDAL_INCLUDE_DIRS})
# add_executable(wsf_network main.cpp)
# target_compile_options(wsf_network PUBLIC -std=c++20)
# target_link_libraries(wsf_network PUBLIC ${GDAL_LIBRARIES} testutil graph_model geometry)