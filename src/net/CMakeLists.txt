cmake_minimum_required(VERSION 2.8)
project (Hrpc_Net)



add_subdirectory("hrpc")

# 包含util头文件 以及 接口相关头文件
include_directories("../util/include")
include_directories("./hrpc")
include_directories("./")

aux_source_directory(. core_src_list)    
aux_source_directory(../util/src util_src_list)

list(APPEND core_src_list ${util_src_list})
# 生成静态库
add_library(hrpc_core STATIC ${core_src_list})

#链接工具库
target_link_libraries(hrpc_core hrpc_util)

# 安装核心网络库
install(TARGETS hrpc_core 
        ARCHIVE DESTINATION hrpc/libstatic)