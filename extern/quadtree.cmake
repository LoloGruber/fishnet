include(ExternalProject)
ExternalProject_Add(
    Quadtree 
    PREFIX ${CMAKE_BINARY_DIR}/quadtree
    GIT_REPOSITORY https://github.com/pvigier/Quadtree.git
    GIT_TAG master
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
)