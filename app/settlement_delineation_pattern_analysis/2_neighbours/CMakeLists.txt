set(neighboursTask ${WORKFLOW_NAME}Neighbours)
add_executable(${neighboursTask} 
    FindNeighboursTask.cpp
)
target_link_libraries(${neighboursTask} Fishnet::Fishnet memgraphAdapter taskLib CLI11::CLI11 nlohmann_json::nlohmann_json magic_enum::magic_enum)
