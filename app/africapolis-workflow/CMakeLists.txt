include(${FISHNET_SOURCE_DIR}/extern/cli11.cmake)
add_executable(AfricapolisGraphComponents GraphComponents.cpp)
target_link_libraries(AfricapolisGraphComponents PRIVATE Fishnet::Fishnet Fishnet::Workflow CLI11::CLI11)
install(TARGETS AfricapolisGraphComponents RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(AfricapolisPolygonOutline PolygonOutline.cpp)
target_link_libraries(AfricapolisPolygonOutline PRIVATE Fishnet::Fishnet CLI11::CLI11)
install(TARGETS AfricapolisPolygonOutline RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(AfricapolisGraphConstruction GraphConstruction.cpp)
target_link_libraries(AfricapolisGraphConstruction PRIVATE Fishnet::Fishnet Fishnet::Workflow::Settlement CLI11::CLI11)
install(TARGETS AfricapolisGraphConstruction RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(FishnetClustering SpatialClustering.cpp)
target_link_libraries(FishnetClustering PRIVATE Fishnet::Fishnet Fishnet::Workflow::Settlement CLI11::CLI11)
install(TARGETS FishnetClustering RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)