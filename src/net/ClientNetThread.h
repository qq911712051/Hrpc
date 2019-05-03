#ifndef CLIENT_NET_THREAD_H_
#define CLIENT_NET_THREAD_H_
#include <mutex>

#include <hrpc_thread.h>
#include <hrpc_epoller.h>
#include <hrpc_queue.h>
#include <hrpc_uid.h>
#include <hrpc_config.h>

#include <ClientConnection.h>
#include <EpollerServer.h>
#include <common.h>
namespace Hrpc
{


class ClientNetThreadGroup;

/**
 * 处理客户端网络收发任务的线程
 */
class ClientNetThread : public Hrpc_Thread
{
    using ConnectionList = std::map<std::string, int>;
public:

    /**
     * @description: 构造函数
     * @param {type} 
     * @return: 
     */
    ClientNetThread(ClientNetThreadGroup* group);

    /**
     * @description: 析构函数
     * @param {type} 
     * @return: 
     */
    ~ClientNetThread();

    /**
     * @description: 初始化网络线程
     * @param {type} 
     * @return: 
     */
    void intialize(const Hrpc_Config& config);

    /**
     * @description: 网络线程运行函数
     * @param {type} 
     * @return: 
     */
    void run() override;

    /**
     * @description: 链接到目的ip端口
     * @param {type} 
     * @return: 返回新建立的链接
     */
    ClientConnectionPtr connect(const std::string& object, const std::string& ip, short port);

    /**
     * @description: 根据uid删除connection
     * @param {type} 
     * @return: 
     */
    void closeConnection(int uid);

    /**
     * @description: 将一个框架信息包 插入到网络线程处理队列
     * @param {type} 
     * @return: 
     */
    void insertResponseQueue(ResponsePtr&& resp);

    /**
     * @description: 终止网络线程执行
     * @param {type} 
     * @return: 
     */
    void terminate();

private:
    /**
     * @description: 接受数据时的回调函数
     * @param {type} 
     * @return: 
     */
    void recvCallback(EpollerServer* server, const ConnectionPtr& ptr);

    /**
     * @description: 关闭epollServer回调
     * @param {type} 
     * @return: 
     */
    void closeCallback(EpollerServer* server) {}

    /**
     * @description: 建立一个链接
     * @param: ip 地址
     * @param: port 端口
     * @return:  返回新连接的fd
     */
    int buildConnection(const std::string& ip, short port);

    /**
     * @description: 将ptr添加到epollServer中
     * @param {type} 
     * @return: 
     */
    void addConnection(const ConnectionPtr& ptr);
    
    /**
     * @description: 检查链接的活动性， 将空闲链接删除
     * @param {type} 
     * @return: 
     */
    void checkActivity();
private:
    ClientNetThreadGroup*           _threadGroup;   // 管理当前网络线程的线程组
    EpollerServer                   _server;        // epollServer核心类
    

    size_t                          _maxIdleTime = {2000};  // 一个链接的最大空闲时间

    size_t                          _max_wait_num = {512};  // 每一条链接上面最大的等待结果返回的数量

    ConnectionList                  _list;  // 代理对象对应的connection的uid
    std::mutex                      _lock;  // 保护_list的线程安全 

    bool                            _terminate = {false};     // 网络线程是否停止
};

using ClientNetThreadPtr = std::unique_ptr<ClientNetThread>;

}
#endif