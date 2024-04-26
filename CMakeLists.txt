cmake_minimum_required(VERSION 3.16)
project(fishnet_project VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
option(FISHNET_Tests "Enable unit tests for Fishnet components" ON)
set(FISHNET_Apps ON  CACHE INTERNAL "Build applications")
add_subdirectory(extern)
add_subdirectory(lib)
if(FISHNET_Apps)
    add_subdirectory(app)
endif()
if(FISHNET_Tests)
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG release-1.12.1
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(tests)
endif()

