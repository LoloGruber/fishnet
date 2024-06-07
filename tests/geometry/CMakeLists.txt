add_subdirectory(geometryTestUtils)

add_executable(geometryTest 
Vec2DTest.cpp
LineTest.cpp
SegmentTest.cpp
RayTest.cpp
RingTest.cpp
SimplePolygonTest.cpp
PolygonTest.cpp
MultiPolygonTest.cpp
kNearestNeighboursTest.cpp
SweepLineTest.cpp
PolygonNeighboursTest.cpp
#CharacteristicShapeTest.cpp
)
gtest_discover_tests(geometryTest)

target_link_libraries(geometryTest PRIVATE geometry testutil geometryTestUtils util_console io)