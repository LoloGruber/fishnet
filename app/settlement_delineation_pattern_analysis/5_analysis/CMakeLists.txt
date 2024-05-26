set(analysisTask ${WORKFLOW_NAME}Analysis)
add_executable(${analysisTask} 
    AnalysisTask.cpp
)
target_link_libraries(${analysisTask} Fishnet::Fishnet memgraphAdapter taskLib CLI11::CLI11 nlohmann_json::nlohmann_json magic_enum::magic_enum)
