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
#include <hrpc_ptr.h>
#include <hrpc_timer.h>
#include <hrpc_queue.h>
#include <hrpc_uid.h>

#include <TcpConnection.h>
#include <BindAdapter.h>

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
class NetThread : Hrpc_Thread
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
    NetThread(NetThreadGroup* ptr, int maxConn = 1024, int wait = 10);

    /**
     * @description: 网络核心线程run函数， 一个主循环监听fd 
     * @param {type} 
     * @return: 
     */
    void run() override;

    /**
     * @description: 初始化整个网络线程 
     * @param {type} 
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
     * @description: 将新的connection加入到connection的map中
     *            
     * @param {type} 
     * @return: 
     */
    void addConnection(const TcpConnectionPtr& ptr);

    /**
     * @description: 将网络线程从epoll_wait中唤醒 
     * @param {type} 
     * @return: 
     */
    void notify();

    /**
     * @description: 停止网络线程的运行
     * @param {type} 
     * @return: 
     */
    void terminate();


private:
    

    /**
     * @description: 处理监听端口事件 
     * @param: uid BindAdapter对应的uid号
     * @return: 
     */
    void acceptConnection(int uid);

    /**
     * @description: 处理来自客户端链接的事件
     * @param: uid  connection对应的uid号
     * @return: 
     */
    void processConnection(int uid);

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
    int                             _waitTime;      // epoll_wait时等待的时间

    response_queue_type             _response_queue;    // 网络线程待处理队列

    Hrpc_Timer                      _timer;         // 定时器， 每次epoll_wait结束以后检测当前timer是否有时间发生.

    Hrpc_Socket                     _shutdown;      // 用于终止网络线程运行

    Hrpc_Socket                     _notify;        // 唤醒阻塞在epoll_wait中的网络线程
    
    std::map<int, TcpConnectionPtr> _connections;   // uid对应的TcpConnection
    std::map<int, BindAdapter*>   _listeners;     // uid对应的BindAdapter

    UidGenarator                    _uidGen;        // uid生成器
    Hrpc_ThreadLock                 _lock;
    const size_t                    _Max_connections;   // 最大连接数
    bool                            _terminate;     // 网络线程是否停止
};

using NetThreadPtr = std::unique_ptr<NetThread>;
}
#endif