set(filterTask ${WORKFLOW_NAME}Filter)
add_executable(${filterTask}
        SettlementFilterTask.cpp
)
target_link_libraries(${filterTask} Fishnet::Fishnet nlohmann_json::nlohmann_json CLI11::CLI11 taskLib magic_enum::magic_enum)
