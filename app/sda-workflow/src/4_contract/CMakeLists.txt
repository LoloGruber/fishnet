set(contractTask ${WORKFLOW_NAME}Contraction)
add_executable(${contractTask} 
    ContractionTask.cpp
)
target_link_libraries(${contractTask} Fishnet::Fishnet WorkflowLib)
install(TARGETS ${contractTask} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)