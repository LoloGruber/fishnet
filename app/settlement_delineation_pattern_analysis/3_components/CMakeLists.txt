set(componentsTask ${WORKFLOW_NAME}Components)
add_executable(${componentsTask} 
    ConnectedComponentsTask.cpp
)
target_link_libraries(${componentsTask} Fishnet::Fishnet memgraphAdapter taskLib nlohmann_json::nlohmann_json CLI11::CLI11)
