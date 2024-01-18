cmake_minimum_required(VERSION 3.16)
project(wsf_network_project)
set(CMAKE_CXX_STANDARD 20)
# set(Boost_USE_STATIC_LIBS OFF)
# set(Boost_USE_MULTITHREADED ON)
# set(Boost_USE_STATIC_RUNTIME OFF)
set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(ENABLE_TESTING ON)

if (MSVC)
    # warning level 4 
    add_compile_options(/W4)
else()
    # gcc / clang flags
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wno-unused-parameter -Wno-unused-function -fconcepts-diagnostics-depth=4)
endif()

#Include Boost
# find_package(Boost 1.71.0 COMPONENTS filesystem)
#Include CGAL
find_package(CGAL REQUIRED)

add_subdirectory(lib)
add_subdirectory(app)
if(ENABLE_TESTING)
    include(FetchContent)
    FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(tests)
endif()



#Include GDAL
#find_package(GDAL REQUIRED)

#include_directories(include ${GDAL_INCLUDE_DIRS})

# add to exe via libary




#Link libraries with exe
#target_link_libraries(wsf_network PUBLIC ${GDAL_LIBRARIES} Boost::filesystem CGAL::CGAL graph geometry io util)

##target_include_directories(wsf_network PUBLIC ${PROJECT_SOURCE_DIR}/util${PROJECT_SOURCE_DIR}/geometry)




