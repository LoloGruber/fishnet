add_library(util_console_concepts INTERFACE)
target_include_directories(util_console_concepts INTERFACE ${CMAKE_CURRENT_LIST_DIR}/concepts)

add_library(util_console STATIC
ProgressPrinter.cpp
StopWatch.cpp
)
target_include_directories(util_console PUBLIC ${CMAKE_CURRENT_LIST_DIR})
