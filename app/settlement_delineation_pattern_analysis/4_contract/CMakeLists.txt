set(neighboursTask ${WORKFLOW_NAME}Contraction)
add_executable(${neighboursTask} 
    ContractionTask.cpp
)
target_link_libraries(${neighboursTask} Fishnet::Fishnet memgraphAdapter taskLib CLI11::CLI11 nlohmann_json::nlohmann_json magic_enum::magic_enum)
