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
DAGTest.cpp
)
gtest_discover_tests(graphTest)
add_executable(contractionPerformance ContractionPerformance.cpp)
target_link_libraries(graphTest PRIVATE graph testutil graphTest_util util_console)
target_link_libraries(contractionPerformance PRIVATE graph testutil graphTest_util util_console)