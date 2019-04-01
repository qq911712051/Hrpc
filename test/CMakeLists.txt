cmake_minimum_required(VERSION 2.8)
project(Hrpc_test)



# 设置-g编译
set(CMAKE_CXX_FLAGS " -g  ${CMAKE_CXX_FLAGS}")

link_libraries("pthread")
## 所有需要测试的目录都在这里添加

add_subdirectory(test_epoll)        # 对Hrpc_Epoller以及Hrpc_Socket以及Hrpc_Exception类进行测试
add_subdirectory(test_thread_lock)  # 对于Hrpc_Thread以及相关锁， 原子变量等进行测试
add_subdirectory(test_util)         # 对于util进行测试