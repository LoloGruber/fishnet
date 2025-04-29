set(FISHNET_WORKFLOW_LIBRARY fishnet-workflow)
set(SUBDIRECTORIES 
graph-database-adapter
task-config
)

# Gather all subdirectory targets starting with $FISHNET_WORKFLOW_LIBRARY
set(FISHNET_WORKFLOW_TARGETS "")
foreach(subdir IN LISTS SUBDIRECTORIES)
    add_subdirectory(${subdir})
    get_property(TARGETS_IN_SUBDIR DIRECTORY ${subdir} PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS TARGETS_IN_SUBDIR)
        if(target MATCHES "^${FISHNET_WORKFLOW_LIBRARY}.*")
            list(APPEND FISHNET_WORKFLOW_TARGETS ${target})
        endif()
    endforeach()
endforeach()

# Create Fishnet workflow library and alias
add_library(${FISHNET_WORKFLOW_LIBRARY} INTERFACE)
target_link_libraries(${FISHNET_WORKFLOW_LIBRARY} INTERFACE ${FISHNET_WORKFLOW_TARGETS})
add_library(${FISHNET_NAME}::Workflow ALIAS ${FISHNET_WORKFLOW_LIBRARY})