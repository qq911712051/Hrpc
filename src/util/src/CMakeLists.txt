cmake_minimum_required(VERSION 2.8)

project("HRPC_UTIL_SOURCE")

include_directories(../include)

# 所有的源文件
aux_source_directory(. Src_List)

# 生成静态库
add_library(hrpc_util STATIC ${Src_List})

# 安装到相关位置
install(TARGETS hrpc_util
        ARCHIVE DESTINATION hrpc/libstatic)
