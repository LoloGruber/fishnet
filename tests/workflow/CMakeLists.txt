add_subdirectory(sda-workflow)
add_executable(workflowTest
MemgraphAdjacencyTest.cpp
PolygonDistanceTest.cpp
CipherQueryTest.cpp
)
gtest_discover_tests(workflowTest)
target_link_libraries(workflowTest PRIVATE Fishnet::Workflow testutil geometryTestUtils graph io) 
