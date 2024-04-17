set(filter ${WORKFLOW_NAME}Filter)
add_executable(${filter} SettlementFilterTask.cpp
        AreaFilter.cpp
        AreaFilter.hpp)
target_link_libraries(${filter} Fishnet::Fishnet nlohmann_json::nlohmann_json)