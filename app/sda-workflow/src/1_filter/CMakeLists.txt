set(filterTask ${WORKFLOW_NAME}Filter)
add_executable(${filterTask}
        SettlementFilterTask.cpp
)
target_link_libraries(${filterTask} Fishnet::Fishnet WorkflowLib)
install(TARGETS ${filterTask} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)