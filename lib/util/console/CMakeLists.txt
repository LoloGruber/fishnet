
add_library(util_console STATIC
        src/ProgressPrinter.cpp
        src/StopWatch.cpp
)
target_include_directories(util_console PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
