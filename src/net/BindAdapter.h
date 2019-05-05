/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description:  抽象端口
 * @Date: 2019-03-30 16:22:41
 */
#ifndef HRPC_BIND_ADAPTER_H_
#define HRPC_BIND_ADAPTER_H_

#include <string>

#include <hrpc_exception.h>
#include <hrpc_queue.h>
#include <hrpc_ptr.h>
#include <hrpc_socket.h>
#include <hrpc_baseProtocol.h>
#include <hrpc_config.h>

#include <TcpConnection.h>
#include <HandleThread.h>

namespace Hrpc
{

class TcpServer;
class NetThreadGroup;

class BindAdapter
{
    using Handle = std::unique_ptr<HandleThread>;
public:

    /**
     * @description: 构造监听socket
     * @param: host 本机地址  
     * @param: port 本机端口 
     * @param: num  端口对应的业务线程数量 
     * @return: 
     */
    BindAdapter(NetThreadGroup* group, const std::string& host, short port, int num, const std::string& objectName);


    /**
     * @description: 释放资源 
     * @param {type} 
     * @return: 
     */
    ~BindAdapter();
    /**
     * @description: 启动所有的业务线程 
     * @param {type} 
     * @return: 
     */
    void start();

    /**
     * @description: 初始化链接
     * @param {type} 
     * @return: 
     */
    void initialize();

    /**
     * @description: 接受新连接
     * @return: 如果接受成功，返回true， 如果失败，返回false
     */
    bool accept();

    /**
     * @description: 插入请求到队列
     * @param {type} 
     * @return: 
     */
    void addRequest(RequestPtr&& req);

    /**
     * @description: 获取所有的请求
     * @param: timeout  过期时间
     * @return: 
     */
    std::queue<RequestPtr> getAllRequest(int timeout = -1);


    /**
     * @description: 获取绑定的本机地址
     * @param {type} 
     * @return: 
     */
    std::string getHost() const {return _host;}

    /**
     * @description: 获取绑定的本机端口
     * @param {type} 
     * @return: 
     */
    short getPort() const {return _port;}

    /**
     * @description: 获取监听端口的套接字 
     * @param {type} 
     * @return: 
     */
    int getBindFd() const {return _listen.getFd();}

    /**
     * @description: 获取端口对应的 对象的名称
     * @param {type} 
     * @return: 
     */
    std::string getObjectName() {return _objectName;}

    /**
     * @description: 往当前端口对应的业务线程中 添加可供解析的 业务协议
     * @param {type} 
     * @return: 
     */
    void addProtocol(Hrpc_BaseProtocol*);

    /**
     * @description: 设置心跳协议 
     * @param {type} 
     * @return: 
     */
    void setHeartProtocol(Hrpc_BaseProtocol*);

    /**
     * @description: 判断业务线程是否正在运行 
     * @param {type} 
     * @return: 
     */
    bool isRunning();
    
private:

    NetThreadGroup*                 _threadGroup;       // 监听当前端口的网络线程
    
    Hrpc_Queue<RequestPtr>          _request_queue;     // 当前端口的请求队列
   
    std::string                     _host;              // 绑定的本机地址
    short                           _port;              // 绑定的本机端口
    Hrpc_Socket                     _listen;            // 端口对应的socket
    
    std::string                     _objectName = {"#"};        // 当前端口虚拟的对象名称
    
    std::vector<Handle>             _handles;           // 业务线程处理组
    int                             _threadNum;         // 业务线程数量
};



}
#endif
