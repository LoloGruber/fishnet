set(contractTask ${WORKFLOW_NAME}Contraction)
add_executable(${contractTask} 
    ContractionTask.cpp
)
target_link_libraries(${contractTask} Fishnet::Fishnet memgraphAdapter taskLib CLI11::CLI11 nlohmann_json::nlohmann_json magic_enum::magic_enum)
install(TARGETS ${contractTask} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)