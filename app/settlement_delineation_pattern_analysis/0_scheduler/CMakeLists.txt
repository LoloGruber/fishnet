add_library(schedulerLib INTERFACE)
target_include_directories(schedulerLib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(schedulerLib INTERFACE Fishnet::Fishnet WorkflowLib)
set(scheduler ${WORKFLOW_NAME}Scheduler)
add_executable(${scheduler}
        Scheduler.cpp
)
target_link_libraries(${scheduler} Fishnet::Fishnet WorkflowLib)
install(TARGETS ${scheduler} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)