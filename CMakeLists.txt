cmake_minimum_required(VERSION 3.16)
project(fishnet_project CXX)
set(CMAKE_CXX_STANDARD 23)
# set(Boost_USE_STATIC_LIBS OFF)
# set(Boost_USE_MULTITHREADED ON)
# set(Boost_USE_STATIC_RUNTIME OFF)
#set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(ENABLE_TESTING ON)

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # gcc / clang flags
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wno-unused-parameter -Wno-unused-function -fconcepts-diagnostics-depth=4)
endif()
add_subdirectory(extern)
add_subdirectory(lib)
add_subdirectory(app)
if(ENABLE_TESTING)
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

