cmake_minimum_required(VERSION 3.0)

project(pasticciotto)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

option(PASTICCIOTTO_DEBUG "Compile pasticciotto in debug mode." OFF)

if (PASTICCIOTTO_DEBUG)
    set(CMAKE_BUILD_TYPE DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DDBG")
endif ()

add_subdirectory(vm)
add_subdirectory(tests)
add_subdirectory(emulator)
add_subdirectory(polictf)