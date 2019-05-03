/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 这是一个通用的epoller服务器 
 * @Date: 2019-05-01 21:07:48
 */
#ifndef EPOLLERSERVER_H_
#define EPOLLERSERVER_H_
#include <functional>

#include <hrpc_epoller.h>
#include <hrpc_socket.h>
#include <hrpc_queue.h>
#include <hrpc_uid.h>
#include <hrpc_timer.h>

#include <ConnectionBase.h>
#include <common.h>

namespace Hrpc
{

/**
 * 基于epoller的服务器
 */
class EpollerServer
{
public:
    enum
    {
        EPOLL_ET_LISTEN = 1,   // 监听fd上面有事件
        EPOLL_ET_NOTIFY,       // 用于将网络线程从epoll_wait中唤醒
        EPOLL_ET_CLOSE,        // 用于终止网络线程
        EPOLL_ET_NET           // 客户端有数据到来
    };
public:
    using response_queue_type = Hrpc_Queue<ResponsePtr>;

    using CloseEventCallback = std::function<void(EpollerServer*)>;     // 处理close事件的回调函数
    using ListenEventCallback = std::function<void(EpollerServer*, const epoll_event&)>;     // 处理新的链接的回调函数
    using RecvDataCallback = std::function<void(EpollerServer*, const ConnectionPtr&)>;     // 接受来自链接数据
    
public:

    EpollerServer() = default;

    /**
     * @description: 初始化EpollerServer 
     * @param {type} 
     * @return: 
     */
    void init(size_t maxConn = 1024, int waitTime = 10);

    /**
     * @description: 处理新的链接到来
     * @param {type} 
     * @return: 
     */
    void setListenCallback(ListenEventCallback&&);

    /**
     * @description: 处理关闭事件
     * @param {type} 
     * @return: 
     */
    void setCloseCallback(CloseEventCallback&&);

    /**
     * @description: 处理网络数据的收发
     * @param {type} 
     * @return: 
     */
    void setRecvCallback(RecvDataCallback&&);

    /**
     * @description: 主要是epoller循环， 并进行一些定时任务处理 
     * @param {type} 
     * @return: 
     */
    void run();

    /**
     * @description: 将其从epoll_Wait中唤醒 
     * @param {type} 
     * @return: 
     */
    void notify();

    /**
     * @description: 将response包插入到当前网络线程处理队列
     * @param {type} 
     * @return: 
     */
    void insertResponseQueue(ResponsePtr&& ptr);

    /**
     * @description: 添加链接到当前EpollerServer
     * @param: conn     链接
     * @return: 
     */
    void addConnection(const ConnectionPtr& conn);

    /**
     * @description: 添加新的监听端口
     * @param: fd   监听socket的fd
     * @param: data epoll的数据
     * @return: 
     */
    void addListen(int fd, size_t data);

    /**
     * @description: 添加一次性任务 
     * @param: after 多久之后执行
     * @param: f     函数对象
     * @param: args  参数
     * @return: 
     */
    template<typename Func, typename... Args>
    Hrpc_Timer::TimerId addTimerTaskOnce(size_t after, Func&& f, Args&&... args);

    /**
     * @description: 添加定时任务 
     * @param: after 多久之后执行
     * @param: dura  周期
     * @param: f     函数对象
     * @param: args  参数
     * @return: 
     */
    template<typename Func, typename... Args>
    Hrpc_Timer::TimerId addTimerTaskRepeat(size_t after, size_t dura, Func&& f, Args&&... args);

    /**
     * @description: 删除链接
     * @param {type} 
     * @return: 
     */
    void closeConnection(int uid);

    
    /**
     * @description: 返回所有connections的引用
     * @param {type} 
     * @return: 
     */
    std::map<int, ConnectionPtr>& getConnections() {return _connections;}

    /**
     * @description: 通过uid获取链接 
     * @param {type} 
     * @return: 
     */
    ConnectionPtr getConnectionByUid(int uid);
private:
    /**
     * @description: 处理 消息队列中任务
     * @param {type} 
     * @return: 
     */
    void processResponse();

    /**
     * @description: 处理来自客户端链接的事件
     * @param: ev  接受到的事件
     * @return: 
     */
    void processConnection(epoll_event ev);
private:
    Hrpc_Epoller                    _ep;            // 管理当前网络线程的fd

    int                             _waitTime = {10};      // epoll_wait时等待的时间

    response_queue_type             _response_queue;    // 网络线程待处理队列

    Hrpc_Timer                      _timer;         // 定时器， 每次epoll_wait结束以后检测当前timer是否有时间发生.

    Hrpc_Socket                     _shutdown;      // 用于终止网络线程运行

    Hrpc_Socket                     _notify;        // 唤醒阻塞在epoll_wait中的网络线程
    
    std::map<int, ConnectionPtr>    _connections;   // uid对应的TcpConnection

    UidGenarator                    _uidGen;        // uid生成器  

    size_t                          _Max_connections;   // 最大连接数

    ListenEventCallback             _listenCallback;    // 接受新的链接
    CloseEventCallback              _closeCallback;     // 关闭epoller
    RecvDataCallback                _recvCallback;      // 接受链路数据
};

template<typename Func, typename... Args>
Hrpc_Timer::TimerId EpollerServer::addTimerTaskOnce(size_t after, Func&& f, Args&&... args)
{
    auto func = std::bind(std::forward<Func>(f), std::forward<Args>(args)...);
    return _timer.addTaskOnce(after, std::move(func));
}
template<typename Func, typename... Args>
Hrpc_Timer::TimerId EpollerServer::addTimerTaskRepeat(size_t after, size_t dura, Func&& f, Args&&... args)
{
    auto func = std::bind(std::forward<Func>(f), std::forward<Args>(args)...);
    return _timer.addTaskRepeat(after, dura, std::move(func));
}

}
#endif