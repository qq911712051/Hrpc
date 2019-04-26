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
     * @description: 初始化网络线程
     * @return: 
     */
    void initialize();

    /**
     * @description: 启动网络线程 
     * @param {type} 
     * @return: 
     */
    void start();

    /**
     * @description: 为线程组添加监听端口， 所有的监听端口都在1号网络线程处理
     * @param: host 监听的本机地址
     * @param: port 监听的本机端口
     * @return: 
     */
    void addBindAdapter(const std::string& host, short port);

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
    
private:

private:
    std::vector<NetThreadPtr>   _threads;           // 网络线程组       
    size_t                      _thread_num;        // 网络线程数量
    Hrpc_Atomic                 _hash;              // 原子计数量， 采用轮训的方式， 讲socket平均分配在多个网络线程之上  
};
}
#endif