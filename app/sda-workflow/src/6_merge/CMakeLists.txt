set(mergeTask ${WORKFLOW_NAME}Merge)
add_executable(${mergeTask} 
    MergeShapefilesTask.cpp
)
target_link_libraries(${mergeTask} Fishnet::Fishnet Fishnet::SDA_Workflow)
install(TARGETS ${mergeTask} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)