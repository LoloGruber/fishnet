set(ConsoleLib fishnet_io_console)
add_library(${ConsoleLib} STATIC
        src/ProgressPrinter.cpp
        src/StopWatch.cpp
)
target_include_directories(${ConsoleLib} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
add_library(${FISHNET}::IO::Console ALIAS ${ConsoleLib})
