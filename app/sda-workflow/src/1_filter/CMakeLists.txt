set(filterTask ${WORKFLOW_NAME}Filter)
add_executable(${filterTask}
        SettlementFilterTask.cpp
)
target_link_libraries(${filterTask} Fishnet::Fishnet Fishnet::SDA_Workflow)
install(TARGETS ${filterTask} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)