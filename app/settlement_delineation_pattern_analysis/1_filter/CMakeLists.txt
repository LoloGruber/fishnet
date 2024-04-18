set(filter ${WORKFLOW_NAME}Filter)
add_executable(${filter}
        SettlementFilterTask.cpp
        ProjectedAreaFilter.hpp
        Filter.hpp
        ApproxAreaFilter.hpp
)
target_link_libraries(${filter} Fishnet::Fishnet nlohmann_json::nlohmann_json CLI11::CLI11)