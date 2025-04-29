set(neighboursTask ${WORKFLOW_NAME}Neighbours)
add_executable(${neighboursTask} 
    FindNeighboursTask.cpp
)
target_link_libraries(${neighboursTask} Fishnet::Fishnet Fishnet::SDA_Workflow)
install(TARGETS ${neighboursTask} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)