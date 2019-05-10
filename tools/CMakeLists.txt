cmake_minimum_required(VERSION 2.8)
project(tools)

set(CMAKE_CXX_FLAGS " -g  -std=c++11 ${CMAKE_CXX_FLAGS}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin/tools/")

include_directories("/usr/local/hrpc/include")
include_directories("./")


link_directories("/usr/local/hrpc/libstatic")

set(SRC_LIST scanner.cpp main.cpp parser.cpp)
add_executable(test ${SRC_LIST})

target_link_libraries(test hrpc_util)

