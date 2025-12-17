set(SDAWorkflowTestBin Test-SDA-Workflow)
add_executable(${SDAWorkflowTestBin}
FilterTest.cpp
JobAdjacencyTest.cpp
ConcurrentSessionsTest.cpp
)
gtest_discover_tests(${SDAWorkflowTestBin})
target_link_libraries(${SDAWorkflowTestBin} PRIVATE Fishnet::SDA_Workflow Fishnet::Workflow::Test geometryTestUtils ${FISHNET}::Graph) 