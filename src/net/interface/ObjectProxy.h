#ifndef OBJECT_PROXY_H_
#define OBJECT_PROXY_H_

#include <hrpc_buffer.h>

namespace Hrpc
{

class ClientNetThreadGroup;

/**
 * 代理对象的基类
 */
class ObjectProxy
{
    enum
    {
        HRPC_ONEWAY = 1,    // 单向调用请求
        HRPC_FUNC           // 正常请求
    };
public:
    /**
     * @description: 构造代理对象
     * @param: group 网络线程组
     * @param: ip    远程对象的ip
     * @param: port  远程对象的端口
     * @return: 
     */
    ObjectProxy(ClientNetThreadGroup* group, const std::string& ip, short port);
    
    /**
     * @description: 进行函数调用， 将请求发往网络线程
     *             并进行一定成都的封装
     * @param: type  调用类型
     * @param: funcName  被调用函数的名称
     * @param: para  将函数参数序列化后的结果
     * @return: 
     */
    Hrpc_Buffer involve(int type, const std::string& funcName, Hrpc_Buffer&& para);    
private:
    ClientNetThreadGroup*   _net;   // 网络线程组
    std::string             _object;    // 代理对象名称
    std::string             _destIp;    // 服务端对象所在的ip
    short                   _destPort;  // 服务端对象所在的端口
};
}
#endif