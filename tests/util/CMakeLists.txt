set(UtilityTestBin Test-Utility)
add_executable(${UtilityTestBin}
BlockingQueueTest.cpp
AlternativeKeyMapTest.cpp
NestedMapTest.cpp
PathHelperTest.cpp
TemporaryDirectoryTest.cpp
)
gtest_discover_tests(${UtilityTestBin})
target_link_libraries(${UtilityTestBin} PRIVATE  ${FISHNET}::Test ${FISHNET}::Collections ${FISHNET}::IO::Filesystem) 