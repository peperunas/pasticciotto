include_directories(include)

add_subdirectory(vm)
add_subdirectory(vmas)

add_executable(pasticciotto-tests test_main.cpp)
target_link_libraries(pasticciotto-tests vm test_vm test_vmas)