cmake_minimum_required(VERSION 3.16)
project(fishnet_project VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
option(FISHNET_TEST "Enable unit tests for Fishnet components" ON)
option(FISHNET_COVERAGE "Enable coverage reporting" OFF)
set(FISHNET_APPS ON  CACHE INTERNAL "Build applications")
set(FISHNET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "Build applications")
set(FISHNET_NAME Fishnet)
add_subdirectory(lib)
if(FISHNET_APPS)
    set(FISHNET_BINARY_INSTALL_DIR ~/.local/bin)
    add_subdirectory(app)
endif()
if(FISHNET_COVERAGE)
    # Add coverage compile flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage -fprofile-update=atomic")
    link_libraries(gcov)
endif()
if(FISHNET_TEST)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG release-1.12.1
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(tests)
endif()

