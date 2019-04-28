cmake_minimum_required(VERSION 2.8)
project(test_thread_lock)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin/test/test_thread_lock/")

include_directories("/usr/local/hrpc/include")
link_directories("/usr/local/hrpc/libstatic")
add_executable(test main.cpp)


target_link_libraries(test hrpc_util)