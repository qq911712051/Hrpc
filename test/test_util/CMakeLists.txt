cmake_minimum_required(VERSION 2.8)
project(test_thread_lock)

aux_source_directory("${CMAKE_SOURCE_DIR}/../src/util/src" UTIL_FILES)


set(LIB_FILES ${UTIL_FILES})

list(APPEND LIB_FILES main.cpp)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin/test/test_util/")

include_directories("${CMAKE_SOURCE_DIR}/../src/util/include/")

add_executable(test_util ${LIB_FILES})