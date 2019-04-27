cmake_minimum_required(VERSION 2.8)
project(HrpcUtil)


add_subdirectory("src")

# 安装头文件到安装目录
install(DIRECTORY include/ 
        DESTINATION hrpc/include/util/)
