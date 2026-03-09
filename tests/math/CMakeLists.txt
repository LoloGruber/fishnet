set(MathTestBin Test-Math)
add_executable(${MathTestBin} 
NormalizeTest.cpp
DegreeTest.cpp
RadiansTest.cpp
StatisticsTest.cpp
)
gtest_discover_tests(${MathTestBin})

target_link_libraries(${MathTestBin} PRIVATE ${FISHNET}::Math Fishnet::Test)