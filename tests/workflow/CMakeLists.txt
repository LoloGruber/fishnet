add_subdirectory(sda-workflow)
add_library(WorkflowTestLib INTERFACE)
target_include_directories(WorkflowTestLib INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(WorkflowTestLib INTERFACE ${FISHNET}::Workflow ${FISHNET}::${FISHNET} ${FISHNET}::Test)
add_library(${FISHNET}::Workflow::Test ALIAS WorkflowTestLib)

add_executable(workflowTest
MemgraphTest.cpp
PolygonDistanceTest.cpp
CipherQueryTest.cpp
)
gtest_discover_tests(workflowTest)
target_link_libraries(workflowTest PRIVATE ${FISHNET}::Workflow ${FISHNET}::Workflow::Test  geometryTestUtils ${FISHNET}::${FISHNET}) 
