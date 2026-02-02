add_subdirectory(sda-workflow)
set(WorkflowTestBin Test-Workflow)
add_executable(${WorkflowTestBin}
MemgraphTest.cpp
PolygonDistanceTest.cpp
CipherQueryTest.cpp
)
gtest_discover_tests(${WorkflowTestBin})
target_link_libraries(${WorkflowTestBin} PRIVATE  ${FISHNET}::${FISHNET} ${FISHNET}::Workflow ${FISHNET}::Test geometryTestUtils ) 
