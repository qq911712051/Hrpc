cmake_minimum_required(VERSION 2.8)
project(Hrpc_test)



# 设置-g编译
set(CMAKE_CXX_FLAGS " -g  -std=c++11 ${CMAKE_CXX_FLAGS}")

link_libraries("pthread")
## 所有需要测试的目录都在这里添加

add_subdirectory(HelloWorld)       # 对网络框架进行测试