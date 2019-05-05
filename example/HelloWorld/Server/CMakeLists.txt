cmake_minimum_required(VERSION 2.8)
project(test_server)



include_directories("/usr/local/hrpc/include")
include_directories("./")
include_directories("../")

link_directories("/usr/local/hrpc/libstatic")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin/example/HelloWorld/")
add_executable(myserver main.cpp)


target_link_libraries(myserver hrpc_util hrpc_core)