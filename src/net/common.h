/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 一些结构体以及类
 * @Date: 2019-03-30 17:09:37
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <memory>
#include <functional>

namespace Hrpc
{

class TcpConnection;

/**
 * 服务器端
 * 业务线程处理的请求包结构体
 */
struct RequestMessage
{
    /**
     *  请求包的类型 
     */
    enum 
    {
        HRPC_REQUEST_HEART = 1,     // 由自身网络线程发送的心跳请求
        HRPC_REQUEST_FUNC           // 由对端 发送的请求
    };
public:
    std::weak_ptr<TcpConnection>    _connection;    // 标识链接
    int                             _type;          // 类别。正常的客户端请求， 网络线程发送的心跳包等等
    size_t                          _stamp;         // 接收请求的时间戳  
    std::unique_ptr<Hrpc_Buffer>    _buffer;        // 请求包的内容
};

/**
 *  网络线程待处理队列中处理的框架内部消息包
 */
struct ResponseMessage
{
    enum 
    {
        HRPC_RESPONSE_NET = 1,  // 当前链接正常的回包
        HRPC_RESPONSE_CLOSE,    // 关闭当前链接
        HRPC_RESPONSE_TASK,     // 异步完成完成某个任务
    };
    using Task = std::function<void()>;
public:
    int                             _uid;           // 链接的uid
    int                             _type;          // 响应类型
    std::unique_ptr<Task>           _task;        // 具体的任务
    std::unique_ptr<Hrpc_Buffer>    _buffer;        // 响应包内容
};

using RequestPtr = std::unique_ptr<RequestMessage>;

using ResponsePtr = std::unique_ptr<ResponseMessage>;

}

#endif
