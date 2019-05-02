#ifndef CLIENT_NET_THREAD_GROUP_H_
#define CLIENT_NET_THREAD_GROUP_H_

#include <hrpc_config.h>
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
    ClientNetThreadGroup(const Hrpc_Config& config);

    /**
     * @description: 初始化整个网络线程组 
     * @param {type} 
     * @return: 
     */
    void intialize();

    /**
     * @description: 启动所有的网络线程 
     * @param {type} 
     * @return: 
     */
    void start();
private:
};
}
#endif