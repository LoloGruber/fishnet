add_library(util_filesystem STATIC
src/TemporaryDirectory.cpp
src/PathHelper.cpp
)
target_include_directories(util_filesystem PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)