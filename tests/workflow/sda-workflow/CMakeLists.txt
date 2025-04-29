add_executable(sdaWorkflowTest
FilterTest.cpp
JobAdjacencyTest.cpp
ConcurrentSessionsTest.cpp
)
gtest_discover_tests(sdaWorkflowTest)
target_link_libraries(sdaWorkflowTest PRIVATE Fishnet::SDA_Workflow testutil geometryTestUtils graph) 
