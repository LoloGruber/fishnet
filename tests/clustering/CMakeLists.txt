set(ClusteringTestBin Test-Clustering)
add_executable(${ClusteringTestBin}
DBScanTest.cpp
BFSClusteringTest.cpp
)
gtest_discover_tests(${ClusteringTestBin})
target_link_libraries(${ClusteringTestBin} PRIVATE  ${FISHNET}::Test ${FISHNET}::Clustering)