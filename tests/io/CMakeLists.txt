add_executable(ioTest
        VectorLayerTest.cpp
        OGRAdapterTest.cpp
        ShapefileTest.cpp
        GeotiffTest.cpp
        FeatureTest.cpp
        FieldDefinitionTestFactory.hpp
        WGS84Test.cpp
)
gtest_discover_tests(ioTest)
target_link_libraries(ioTest PRIVATE io testutil util_filesystem)