add_executable(utilTest
BlockingQueueTest.cpp
AlternativeKeyMapTest.cpp
NestedMapTest.cpp
PathHelperTest.cpp
TemporaryDirectoryTest.cpp
)
gtest_discover_tests(utilTest)
target_link_libraries(utilTest PRIVATE  testutil util) 