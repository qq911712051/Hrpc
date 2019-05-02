#ifndef CLIENT_NET_THREAD_H_
#define CLIENT_NET_THREAD_H_

#include <hrpc_thread.h>
#include <hrpc_epoller.h>
#include <hrpc_queue.h>
#include <hrpc_uid.h>
#include <hrpc_config.h>

#include <ClientConnection.h>
#include <common.h>
namespace Hrpc
{


class ClientNetThreadGroup;

/**
 * 处理客户端网络收发任务的线程
 */
class ClientNetThread : public Hrpc_Thread
{
    using response_queue_type = Hrpc_Queue<ResponsePtr>;
public:

    /**
     * @description: 构造函数
     * @param {type} 
     * @return: 
     */
    ClientNetThread(ClientNetThreadGroup* group, const Hrpc_Config& config);

    /**
     * @description: 初始化网络线程
     * @param {type} 
     * @return: 
     */
    void intialize();

    /**
     * @description: 网络线程运行函数
     * @param {type} 
     * @return: 
     */
    void run() override;
private:
    ClientNetThreadGroup*           _threadGroup;   // 管理当前网络线程的线程组

    Hrpc_Epoller                    _ep;            // 管理当前网络线程的fd
    int                             _waitTime = {10};      // epoll_wait时等待的时间

    response_queue_type             _response_queue;    // 网络线程待处理队列

    Hrpc_Socket                     _notify;        // 唤醒阻塞在epoll_wait中的网络线程
    
    std::map<int, ClientConnectionPtr> _connections;   // uid对应的Connection

    UidGenarator                    _uidGen;        // uid生成器  

    size_t                          _Max_connections;   // 最大连接数
    bool                            _terminate = {false};     // 网络线程是否停止

};

}
#endif