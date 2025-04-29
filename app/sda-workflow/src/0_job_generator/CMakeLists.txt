add_library(generatorLib INTERFACE)
target_include_directories(generatorLib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(generatorLib INTERFACE Fishnet::Fishnet Fishnet::SDA_Workflow)
set(generator ${WORKFLOW_NAME}JobGenerator)
add_executable(${generator}
        JobGenerator.cpp
)
target_link_libraries(${generator} Fishnet::Fishnet Fishnet::SDA_Workflow)
install(TARGETS ${generator} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)