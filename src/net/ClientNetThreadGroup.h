#ifndef CLIENT_NET_THREAD_GROUP_H_
#define CLIENT_NET_THREAD_GROUP_H_
#include <memory>
#include <vector>

#include <hrpc_config.h>
#include <hrpc_atomic.h>

#include <ClientNetThread.h>
namespace Hrpc
{

class ClientNetThreadGroup
{
public:
    /**
     * @description: 构造函数
     * @param {type} 
     * @return: 
     */
    ClientNetThreadGroup() = default;

    /**
     * @description: 析构函数
     * @param {type} 
     * @return: 
     */
    ~ClientNetThreadGroup();
    /**
     * @description: 初始化整个网络线程组 
     * @param {type} 
     * @return: 
     */
    void intialize(const Hrpc_Config& config);

    /**
     * @description: 启动所有的网络线程 
     * @param {type} 
     * @return: 
     */
    void start();

    /**
     * @description: 轮训的获得一个客户端网络线程
     * @param {type} 
     * @return: 
     */
    ClientNetThread* getNetThreadByRound();

    /**
     * @description: 终止网络线程组运行
     * @param {type} 
     * @return: 
     */
    void terminate();
private:
    std::vector<ClientNetThreadPtr>     _threads;       // 网络线程组
    size_t                              _num;           // 网络线程数量
    Hrpc_Atomic                         _hash;          // 进行轮训时使用
};
}
#endif