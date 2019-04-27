cmake_minimum_required(VERSION 2.8)
project (Hrpc_Net)

aux_source_directory(. net_src_list)

add_subdirectory("interface")

# 包含util头文件 以及 接口相关头文件
include_directories("../util/include")
include_directories("./interface")
include_directories("./")

# 生成静态库
add_library(hrpc_core STATIC ${net_src_list})

#链接工具库
target_link_libraries(hrpc_core hrpc_util)

# 安装核心网络库
install(TARGETS hrpc_core 
        ARCHIVE DESTINATION hrpc/libstatic)