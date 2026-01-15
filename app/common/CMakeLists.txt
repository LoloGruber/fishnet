add_executable(FishnetShapefileSplitter ShapeSplitter.cpp)
target_link_libraries(FishnetShapefileSplitter PRIVATE Fishnet::Fishnet CLI11::CLI11)
install(TARGETS FishnetShapefileSplitter RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(FishnetClustering SpatialClustering.cpp)
target_link_libraries(FishnetClustering PRIVATE Fishnet::Fishnet Fishnet::Workflow CLI11::CLI11)
install(TARGETS FishnetClustering RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(FishnetClearDatabase ClearDatabase.cpp)
target_link_libraries(FishnetClearDatabase PRIVATE Fishnet::Fishnet Fishnet::Workflow CLI11::CLI11)
install(TARGETS FishnetClearDatabase RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)