cmake_minimum_required(VERSION 2.8)
project("Hrpc_API")

# 安装相关接口到 对应位置
install(DIRECTORY ./ 
        DESTINATION hrpc/include/net
        FILES_MATCHING PATTERN "*.h")



