cmake_minimum_required(VERSION 2.8)
project(test_epoll)

aux_source_directory("${CMAKE_SOURCE_DIR}/../src/util/src" UTIL_FILES)


set(CLIENT_FILES ${UTIL_FILES})
set(SERVER_FILES ${UTIL_FILES})

list(APPEND CLIENT_FILES client.cpp)
list(APPEND SERVER_FILES server.cpp)

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin/test/test_epoll/")

include_directories("${CMAKE_SOURCE_DIR}/../src/util/include/")

add_executable(client ${CLIENT_FILES})
add_executable(server ${SERVER_FILES})