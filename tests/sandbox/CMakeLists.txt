add_executable(sandbox Sandbox.cpp CRTPExample.h Concepts.h)
target_link_libraries(sandbox PRIVATE Fishnet::Fishnet)