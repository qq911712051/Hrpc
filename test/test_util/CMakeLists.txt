cmake_minimum_required(VERSION 2.8)
project(test_util)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin/test/test_util/")

include_directories("../../src/util/include")

aux_source_directory("../../src/util/src"  SrcList)

list(APPEND SrcList main.cpp)

add_executable(test_util ${SrcList})

