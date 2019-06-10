add_executable(polictf-client pasticciotto_client.cpp)
target_link_libraries(polictf-client vm)

add_dependencies(polictf polictf-client)