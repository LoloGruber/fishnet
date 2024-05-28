set(analysisTask ${WORKFLOW_NAME}Analysis)
add_executable(${analysisTask} 
    AnalysisTask.cpp
)
target_link_libraries(${analysisTask} Fishnet::Fishnet WorkflowLib)
install(TARGETS ${analysisTask} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)