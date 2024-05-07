set(neighboursTask ${WORKFLOW_NAME}Neighbours)
add_executable(${neighboursTask} 
    FindNeighboursTask.cpp
)
target_link_libraries(${neighboursTask} Fishnet::Fishnet memgraphAdapter taskLib)
