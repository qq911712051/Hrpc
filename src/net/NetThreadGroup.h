/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 网络线程组， 主要处理收发包
 * @Date: 2019-03-31 20:31:24
 */
#ifndef HRPC_NETTHREADGROUP_H_
#define HRPC_NETTHREADGROUP_H_
#include <vector>

#include <hrpc_atomic.h>
#include <hrpc_config.h>

#include <NetThread.h>
namespace Hrpc
{
class NetThreadGroup
{
public:
    NetThreadGroup() = default;

    /**
     * @description: 析构函数, 释放资源 
     * @param {type} 
     * @return: 
     */
    ~NetThreadGroup();

    /**
     * @description: 初始化网络线程组
     *       新建网络线程
     * @return: 
     */
    void initialize(const Hrpc_Config& config);

    /**
     * @description: 启动网络线程 
     *      初始化所有网络线程并启动
     * @param {type} 
     * @return: 
     */
    void start();

    /**
     * @description: 添加监听端口
     * @param: bind 端口对象的指针
     * @return: 
     */
    void addBindAdapter(BindAdapter* bind);

    /**
     * @description: 采用轮训策略，选出一个新的网络线程
     * @param {type} 
     * @return: 返回被选择的网络线程
     */
    NetThread* getNetThreadByRound();

    /**
     * @description: 终止网络线程执行 
     * @param {type} 
     * @return: 
     */
    void terminate();

    /**
     * @description: 网络线程组是否正在运行
     * @param {type} 
     * @return: 
     */
    bool isRunning();
    
private:

private:
    std::vector<NetThreadPtr>   _threads;           // 网络线程组       
    size_t                      _thread_num;        // 网络线程数量
    Hrpc_Atomic                 _hash;              // 原子计数量， 采用轮训的方式， 讲socket平均分配在多个网络线程之上  
};
}
#endif