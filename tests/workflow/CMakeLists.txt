add_executable(workflowTest
FilterTest.cpp
MemgraphTest.cpp
JobAdjacencyTest.cpp
PolygonDistanceTest.cpp
)
gtest_discover_tests(workflowTest)
target_link_libraries(workflowTest PRIVATE WorkflowLib testutil geometryTestUtils graph) 
