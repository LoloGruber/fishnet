add_executable(FishnetShapefileSplitter ShapefileSplitter.cpp)
target_link_libraries(FishnetShapefileSplitter PRIVATE Fishnet::Fishnet CLI11::CLI11)
install(TARGETS FishnetShapefileSplitter RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(FishnetShapefileMerger ShapefileMerger.cpp)
target_link_libraries(FishnetShapefileMerger PRIVATE Fishnet::Fishnet CLI11::CLI11)
install(TARGETS FishnetShapefileMerger RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(FishnetShapefilePreprocessor ShapefilePreprocessor.cpp)
target_link_libraries(FishnetShapefilePreprocessor PRIVATE Fishnet::Fishnet Fishnet::Workflow CLI11::CLI11)
install(TARGETS FishnetShapefilePreprocessor RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(FishnetClearDatabase ClearDatabase.cpp)
target_link_libraries(FishnetClearDatabase PRIVATE Fishnet::Fishnet Fishnet::Workflow CLI11::CLI11)
install(TARGETS FishnetClearDatabase RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)