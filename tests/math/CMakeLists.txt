add_executable(mathTest 
NormalizeTest.cpp
DegreeTest.cpp
RadiansTest.cpp
        WGS84DistanceTest.cpp
)
gtest_discover_tests(mathTest)

target_link_libraries(mathTest PRIVATE math testutil)