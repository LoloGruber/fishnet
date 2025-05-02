include(${FISHNET_SOURCE_DIR}/extern/cli11.cmake)
add_executable(AfricapolisGraphComponents AfricapolisGraphComponents.cpp)
target_link_libraries(AfricapolisGraphComponents PRIVATE Fishnet::Fishnet Fishnet::Workflow CLI11::CLI11)
install(TARGETS AfricapolisGraphComponents RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

add_executable(AfricapolisClearDatabase ClearDatabase.cpp)
target_link_libraries(AfricapolisClearDatabase PRIVATE Fishnet::Fishnet Fishnet::Workflow CLI11::CLI11)
install(TARGETS AfricapolisClearDatabase RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
