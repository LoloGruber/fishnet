add_executable(FishnetShapefileSplitter ShapeSplitter.cpp)
target_link_libraries(FishnetShapefileSplitter PRIVATE Fishnet::Fishnet CLI11::CLI11)
install(TARGETS FishnetShapefileSplitter RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)