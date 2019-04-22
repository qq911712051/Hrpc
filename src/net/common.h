/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 一些结构体以及类
 * @Date: 2019-03-30 17:09:37
 */

#ifndef HRPC_COMMON_H_
#define HRPC_COMMON_H_

namespace Hrpc
{

/**
 *  请求包的类型 
 */
enum 
{
    HRPC_REQUEST_HEART = 1,     // 心跳协议
    HRPC_REQUEST_FUNC           // 普通函数请求
};



}

#endif
