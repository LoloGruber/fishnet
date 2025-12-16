set(IOTestBin Test-IO)
add_executable(${IOTestBin}
        VectorLayerTest.cpp
        OGRAdapterTest.cpp
        ShapefileTest.cpp
        GeotiffTest.cpp
        FeatureTest.cpp
        FieldDefinitionTestFactory.hpp
        WGS84Test.cpp
        GeoPackageTest.cpp
)
gtest_discover_tests(${IOTestBin})
target_link_libraries(${IOTestBin} PRIVATE ${FISHNET}::IO Fishnet::Test)