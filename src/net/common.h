/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 一些结构体以及类
 * @Date: 2019-03-30 17:09:37
 */

#ifndef HRPC_COMMON_H_
#define HRPC_COMMON_H_

#include <memory>
#include <functional>

namespace Hrpc
{


/**
 * 业务线程处理的请求包结构体
 */
struct RequestMessage
{
    /**
     *  请求包的类型 
     */
    enum 
    {
        HRPC_REQUEST_HEART = 1,     // 心跳协议
        HRPC_REQUEST_FUNC           // 普通函数请求
    };
public:
    std::weak_ptr<TcpConnection>    _connection;    // 标识链接
    int                             _type;          // 类别。正常的客户端请求， 网络线程发送的心跳包等等
    size_t                          _stamp;         // 接收请求的时间戳  
    std::unique_ptr<Hrpc_Buffer>    _buffer;        // 请求包的内容
};

/**
 * @description: 网络线程需要处理的响应包  
 */
struct ResponseMessage
{
    /**
     * @description: 响应包的类型
     */
    enum 
    {
        HRPC_RESPONSE_NET = 1,  // 当前链接正常的回包
        HRPC_RESPONSE_CLOSE,    // 关闭当前链接
        HRPC_RESPONSE_TASK,     // 在此网络线程异步完成完成某个任务
    };
    using Task = std::function<void()>;
public:
    std::weak_ptr<TcpConnection>    _connection;    // 标识链接
    int                             _type;          // 响应类型
    std::unique_ptr<Task>           _task;        // 具体的任务
    std::unique_ptr<Hrpc_Buffer>    _buffer;        // 响应包内容
};

using RequestPtr = std::unique_ptr<RequestMessage>;

using ResponsePtr = std::unique_ptr<ResponseMessage>;


}

#endif
