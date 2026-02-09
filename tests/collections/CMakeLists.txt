set(CollectionsTestBin Test-Collections)
add_executable(${CollectionsTestBin}
BlockingQueueTest.cpp
AlternativeKeyMapTest.cpp
BidirectionalHashMapTest.cpp
BidirectionalHashMultiMapTest.cpp
NestedMapTest.cpp
)
gtest_discover_tests(${CollectionsTestBin})
target_link_libraries(${CollectionsTestBin} PRIVATE  ${FISHNET}::Test ${FISHNET}::Collections) 