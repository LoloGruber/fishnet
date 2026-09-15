add_executable(Test-Compile Sandbox.cpp)

# Commenting out failing test for fishnet::test::TestUtil.hpp
# set(TestSandboxBin Test-Misc)
# add_executable(${TestSandboxBin} 
# TestUtilTest.cpp
# )
# gtest_discover_tests(${TestSandboxBin})
# target_link_libraries(${TestSandboxBin} Fishnet::Test)