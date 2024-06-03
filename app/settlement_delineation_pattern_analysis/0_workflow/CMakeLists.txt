set(workflowTask ${WORKFLOW_NAME})
add_executable(${workflowTask}
    SettlementDelineation.cpp
)
target_link_libraries(${workflowTask} Fishnet::Fishnet WorkflowLib)
install(TARGETS ${workflowTask} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)