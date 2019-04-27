cmake_minimum_required(VERSION 2.8)

project(HRPC_SRC)


# 设置-g 选项

set(CMAKE_CXX_FLAGS " -g  -std=c++11 ${CMAKE_CXX_FLAGS}")    


# 链接pthread库
link_libraries("pthread")

add_subdirectory("util")     # 工具库
add_subdirectory("net")     # 工具库

