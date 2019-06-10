add_executable(polictf-server pasticciotto_server.cpp)
target_link_libraries(polictf-server vm)

add_dependencies(polictf polictf-server)
