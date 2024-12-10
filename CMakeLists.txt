cmake_minimum_required(VERSION 3.16)
project(fishnet_project VERSION 1.0.0 LANGUAGES CXX)

# Options
option(FISHNET_TEST "Enable unit tests for Fishnet components" ON)
option(FISHNET_COVERAGE "Enable coverage reporting" OFF)
option(FISHNET_APPS "Build applications" ON)

# Variables
set(FISHNET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Build applications")
set(FISHNET_NAME Fishnet)
set(CMAKE_CXX_STANDARD 23)

# Build core lib
add_subdirectory(lib)

# Build workflows and examples
if(FISHNET_APPS)
    add_subdirectory(app)
endif()

# Enable code coverage
if(FISHNET_COVERAGE)
    # Add coverage compile flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage -fprofile-update=atomic")
    link_libraries(gcov)
endif()

# Build tests
if(FISHNET_TEST)
    include(extern/gtest.cmake)
    enable_testing()
    add_subdirectory(tests)
endif()

