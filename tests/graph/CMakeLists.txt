add_subdirectory(graphTestUtils)

SET(GraphTestBin Test-Graph)
add_executable(${GraphTestBin} 
CentralityTest.cpp 
UndirectedGraphTest.cpp 
DirectedGraphTest.cpp 
ConncectedComponentsTest.cpp 
SearchPathTest.cpp 
ContractionTest.cpp 
WeightedGraphTest.cpp
EdgeTest.cpp
DAGTest.cpp
JSONAdjacencyTest.cpp
)
gtest_discover_tests(${GraphTestBin})
target_link_libraries(${GraphTestBin} PRIVATE ${FISHNET_NAME}::Graph testutil graphTest_util util_console)

SET(ContractionTestBin Test-Contraction)
add_executable(${ContractionTestBin} ContractionPerformance.cpp)
target_link_libraries(${ContractionTestBin} PRIVATE ${FISHNET_NAME}::Graph testutil graphTest_util util_console)