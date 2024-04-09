set(filter ${WORKFLOW_NAME}Filter)
add_executable(${filter} SettlementFilterTask.cpp)
target_link_libraries(${filter} geometry io util_functional util_filesystem util_console nlohmann_json::nlohmann_json)