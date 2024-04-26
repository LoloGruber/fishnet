add_subdirectory(graphTestUtils)
add_executable(graphTest 
CentralityTest.cpp 
UndirectedGraphTest.cpp 
DirectedGraphTest.cpp 
ConncectedComponentsTest.cpp 
SearchPathTest.cpp 
ContractionTest.cpp 
WeightedGraphTest.cpp
EdgeTest.cpp
MemgraphTest.cpp
)
gtest_discover_tests(graphTest)

add_executable(contractionPerformance ContractionPerformance.cpp)

target_link_libraries(graphTest PRIVATE graph testutil graphTest_util util_console)
target_link_libraries(contractionPerformance PRIVATE graph_model graph_algo testutil graphTest_util util_console)