set(FilesystemLib fishnet_io_filesystem)
add_library(${FilesystemLib} STATIC
src/TemporaryDirectory.cpp
src/PathHelper.cpp
)
target_include_directories(${FilesystemLib} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
add_library(${FISHNET}::IO::Filesystem ALIAS ${FilesystemLib})