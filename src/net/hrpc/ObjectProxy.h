#ifndef OBJECT_PROXY_H_
#define OBJECT_PROXY_H_

#include <future>

#include <hrpc_buffer.h>
#include <hrpc_atomic.h>
namespace Hrpc
{

class ClientNetThreadGroup;

/**
 * 代理对象的基类
 */
class ObjectProxy
{

protected:
    enum
    {
        HRPC_ONEWAY = 1,    // 单向调用请求
        HRPC_FUNC           // 正常请求
    };

    using Future_Data = std::unique_ptr<Hrpc_Buffer>;
    using WaitFuture = std::future<Future_Data>;
public:
    /**
     * @description: 构造代理对象
     * @param: group 网络线程组
     * @param: objectName 代理对象名称
     * @param: ip    远程对象的ip
     * @param: port  远程对象的端口
     * @return: 
     */
    ObjectProxy(ClientNetThreadGroup* group, const std::string& objectName, const std::string& ip, short port, size_t waitTime);
    
    /**
     * @description: 进行函数调用， 将请求发往网络线程
     *             并进行一定成都的封装
     * @param: type  调用类型
     * @param: funcName  被调用函数的名称k
     * @param: para  将函数参数序列化后的结果
     * @return: 
     */
    Hrpc_Buffer involve(int type, const std::string& funcName, Hrpc_Buffer&& para);    

    /**
     * @description: 设置等待时间
     * @param： wait 等待时间 ， 单位ms
     * @return: 
     */
    void setWaitTime(size_t wait);
private:
    /**
     * @description: 封装成一个Hrpc协议的协议体
     * @param {type} 
     * @return: 
     */
    Hrpc_Buffer makeHrpcBody(const std::string& obj, Hrpc_Buffer&& buf);

    /**
     * @description: 对于HRPC_FUNC类型的调用， 等待结果返回后返回结果
     * @param {type} 
     * @return: 
     */
    Hrpc_Buffer waitResult(WaitFuture&& fut);
private:
    ClientNetThreadGroup*   _net;   // 网络线程组
    std::string             _object;    // 代理对象名称
    std::string             _destIp;    // 服务端对象所在的ip
    short                   _destPort;  // 服务端对象所在的端口

    size_t                  _waitTime = {2000}; // 等待时间为2s    
};
}
#endif