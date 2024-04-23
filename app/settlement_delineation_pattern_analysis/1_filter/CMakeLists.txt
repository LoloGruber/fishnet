add_subdirectory(filters)
set(filterLib ${WORKFLOW_NAME}Filter)
add_executable(${filterLib}
        SettlementFilterTask.cpp
)
target_link_libraries(${filterLib} Fishnet::Fishnet nlohmann_json::nlohmann_json CLI11::CLI11 workflowFilters)
