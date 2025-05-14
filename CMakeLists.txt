cmake_minimum_required(VERSION 3.16)
project(fishnet_project VERSION 1.0.0 LANGUAGES CXX)

# Options
option(FISHNET_TEST "Enable unit tests for Fishnet components" ON)
option(FISHNET_APPS "Build applications" ON)
option(FISHNET_COVERAGE "Enable coverage reporting" OFF)
option(FISHNET_COMPILE_TIME_TRACE "Enable compile time tracing" OFF)

# Variables
set(FISHNET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Build applications")
set(FISHNET_NAME Fishnet)
set(CMAKE_CXX_STANDARD 23)

# Compiler flags and warnings
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "Setting compiler flags and warnings")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wno-unused-parameter -Wno-unused-function -fconcepts-diagnostics-depth=2 )
    if(FISHNET_COMPILE_TIME_TRACE)
        add_compile_options(-fplugin=externis -fplugin-arg-externis-trace=${CMAKE_BINARY_DIR}/trace.json)
    endif()
endif()

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

