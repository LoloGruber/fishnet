add_executable(sandbox Sandbox.cpp CRTPExample.h Concepts.h)

set(TestSandboxBin Test-Sandbox)
add_executable(${TestSandboxBin} 
TestUtilTest.cpp
)
gtest_discover_tests(${TestSandboxBin})

target_link_libraries(${TestSandboxBin} Fishnet::Test)