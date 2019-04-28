cmake_minimum_required(VERSION 2.8)
project(test_epoll)


set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin/test/test_epoll/")

include_directories("/usr/local/hrpc/include")


link_directories("/usr/local/hrpc/libstatic")
add_executable(client client.cpp)

add_executable(server server.cpp)



target_link_libraries(client hrpc_util)
target_link_libraries(server hrpc_util)