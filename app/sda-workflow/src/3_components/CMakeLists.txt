set(componentsTask ${WORKFLOW_NAME}Components)
add_executable(${componentsTask} 
    ConnectedComponentsTask.cpp
)
target_link_libraries(${componentsTask} Fishnet::Fishnet Fishnet::SDA_Workflow)
install(TARGETS ${componentsTask} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)