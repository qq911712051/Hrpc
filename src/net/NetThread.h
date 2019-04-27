/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description:  网络线程类，核心线程
 * @Date: 2019-03-30 16:05:58
 */

#ifndef HRPC_NETTHREAD_H_
#define HRPC_NETTHREAD_H_
#include <list>
#include <map>

#include <hrpc_thread.h>
#include <hrpc_socket.h>
#include <hrpc_lock.h>
#include <hrpc_epoller.h>
#include <hrpc_config.h>
#include <hrpc_ptr.h>
#include <hrpc_timer.h>
#include <hrpc_queue.h>
#include <hrpc_uid.h>

#include <TcpConnection.h>
#include <BindAdapter.h>
#include <common.h>

namespace Hrpc
{

class NetThreadGroup;



class Hrpc_NetThreadException : public Hrpc_Exception
{
public:
    Hrpc_NetThreadException(const std::string& str) : Hrpc_Exception(str) {}
    Hrpc_NetThreadException(const std::string& str, int err) : Hrpc_Exception(str, err) {}
    ~Hrpc_NetThreadException() {}
};


/**
 *  网络线程核心类， 主要进行收发包工作
 *     以及验证包的完整性
 */
class NetThread : public Hrpc_Thread
{
    enum
    {
        EPOLL_ET_LISTEN = 1,   // 监听fd上面有事件
        EPOLL_ET_NOTIFY,       // 用于将网络线程从epoll_wait中唤醒
        EPOLL_ET_CLOSE,        // 用于终止网络线程
        EPOLL_ET_NET           // 客户端有数据到来
    };
public:
    typedef Hrpc_Queue<ResponsePtr> response_queue_type;
public:
    
    /**
     * @description: 初始化网络线程
     * @param {type} 
     * @return: 
     */
    NetThread(NetThreadGroup* ptr, const Hrpc_Config& config);
    
    ~NetThread();

    /**
     * @description: 网络核心线程run函数， 一个主循环监听fd 
     * @param {type} 
     * @return: 
     */
    void run() override;

    /**
     * @description: 初始化整个网络线程 
     * @param: config 配置选项
     * @return: 
     */
    void initialize();

    /**
     * @description: 在当前网络线程添加监听端口
     *          注： 此函数必须在initialize初始化函数调用之前调用才会生效
     * @param {type} 
     * @return: 
     */
    void addBindAdapter(BindAdapter* bind);

    /**
     * @description: 将response包插入到当前网络线程处理队列
     * @param {type} 
     * @return: 
     */
    void insertResponseQueue(ResponsePtr&& ptr);


    /**
     * @description: 将新的TcpConnection加入到当前网络线程
     *          添加操作将会由此网络线程执行， 避免加锁
     * @param: ptr 一条网络链接
     * @return: 
     */
    void addConnection(const TcpConnectionPtr& ptr);


    /**
     * @description: 在此线程中异步运行一个任务
     * @param {type} 
     * @return: 
     */
    template <typename Func, typename... Args>
    Hrpc_Timer::TimerId runTaskBySelf(Func&& f, Args&&... args);


    /**
     * @description: 停止网络线程的运行
     * @param {type} 
     * @return: 
     */
    void terminate();

    /**
     * @description: 链接被动关闭， 触发原因可能是
     *             对端关闭链接， 或者是 对端对于心跳没有反应
     * @param
     * @return: 
     */
    void closeConnection(int uid);

private:

    /**
     * @description: 将网络线程从epoll_wait中唤醒 
     * @param {type} 
     * @return: 
     */
    void notify();
    
    /**
     * @description: 心跳检测， 作为一个定时任务执行
     * @param 
     * @return: 
     */
    void HeartCheckTask();



    /**
     * @description: 处理监听端口事件 
     * @param: ev 接受到的事件
     * @return: 
     */
    void acceptConnection(epoll_event ev);

    /**
     * @description: 处理来自客户端链接的事件
     * @param: ev  接受到的事件
     * @return: 
     */
    void processConnection(epoll_event ev);

    /**
     * @description: 触发shutdown事件
     * @param {type} 
     * @return: 
     */
    void closeEvent() {}

    /**
     * @description: 将业务线程的响应包返回到客户端 
     * @param {type} 
     * @return: 
     */
    void processResponse();
private:
    NetThreadGroup*                 _threadGroup;   // 管理当前网络线程的线程组
            
    Hrpc_Epoller                    _ep;            // 管理当前网络线程的fd
    int                             _waitTime = {10};      // epoll_wait时等待的时间
    int                             _heartTime = {2000};     // 心跳检测时间， 一个connection的不活动时间超过_heartTime, 就会对其发送心跳检测

    response_queue_type             _response_queue;    // 网络线程待处理队列

    Hrpc_Timer                      _timer;         // 定时器， 每次epoll_wait结束以后检测当前timer是否有时间发生.

    Hrpc_Socket                     _shutdown;      // 用于终止网络线程运行

    Hrpc_Socket                     _notify;        // 唤醒阻塞在epoll_wait中的网络线程
    
    std::map<int, TcpConnectionPtr> _connections;   // uid对应的TcpConnection
    std::map<int, BindAdapter*>   _listeners;     // uid对应的BindAdapter

    UidGenarator                    _uidGen;        // uid生成器  

    size_t                          _Max_connections;   // 最大连接数
    bool                            _terminate;     // 网络线程是否停止
};

using NetThreadPtr = std::unique_ptr<NetThread>;


template <typename Func, typename... Args>
Hrpc_Timer::TimerId NetThread::runTaskBySelf(Func&& f, Args&&... args)
{
    // 将传入参数封装为函数对象
    auto task = std::bind(std::forward<Func>(f), std::forward<Args>(args)...);
    
    // 新建一个response对象
    auto resp = ResponsePtr(new ResponseMessage);
    resp->_type = ResponseMessage::HRPC_RESPONSE_TASK;
    
    resp->_task = std::unique_ptr<ResponseMessage::Task>(new ResponseMessage::Task(std::move(task)));

    _response_queue.push(std::move(resp));

    // 将网络线程从epoll_wait中唤醒
    notify();
}
}
#endif