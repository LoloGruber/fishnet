include(${FISHNET_SOURCE_DIR}/extern/cli11.cmake)
include(${FISHNET_SOURCE_DIR}/extern/magic_enum.cmake)

add_executable(FishnetVectorFileSplitter VectorFileSplitter.cpp)
target_link_libraries(FishnetVectorFileSplitter PRIVATE Fishnet::Fishnet Fishnet::Workflow CLI11::CLI11 magic_enum::magic_enum)
install(TARGETS FishnetVectorFileSplitter RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(FishnetVectorFileMerger VectorFileMerger.cpp)
target_link_libraries(FishnetVectorFileMerger PRIVATE Fishnet::Fishnet Fishnet::Workflow CLI11::CLI11)
install(TARGETS FishnetVectorFileMerger RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(FishnetVectorFilePreprocessor VectorFilePreprocessor.cpp)
target_link_libraries(FishnetVectorFilePreprocessor PRIVATE Fishnet::Fishnet Fishnet::Workflow CLI11::CLI11)
install(TARGETS FishnetVectorFilePreprocessor RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)