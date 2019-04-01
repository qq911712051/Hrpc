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

#include <TcpConnection.h>

namespace Hrpc
{

class TcpServer;
class NetThreadGroup;

class BindAdapter
{
public:

    /**
     * @description: 构造监听socket
     * @param: host 本机地址  
     * @param: port 本机端口 
     * @return: 
     */
    BindAdapter(NetThreadGroup* group, const std::string& host, short port);

    /**
     * @description: 初始化链接
     * @param {type} 
     * @return: 
     */
    void initialize();

    /**
     * @description: 接受新连接
     * @param: ptr 新连接智能指针
     * @return: 如果接受成功，返回true， 如果失败，返回false
     */
    bool accept(TcpConnectionPtr& ptr);

    /**
     * @description: 插入请求到队列
     * @param {type} 
     * @return: 
     */
    void addRequest(RequestPtr req);

    /**
     * @description: 阻塞获取新请求
     * @param: req 新请求   timeout  超时时间
     * @return: 是否超时
     */
    bool getRequest(RequestPtr& req, int timeout);


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
    
private:

    NetThreadGroup*                 _threadGroup;       // 监听当前端口的网络线程
    
    Hrpc_Queue<RequestPtr>          _request_queue;     // 当前端口的请求队列
   
    std::string                     _host;              // 绑定的本机地址
    short                           _port;              // 绑定的本机端口
    Hrpc_Socket                     _listen;            // 端口对应的socket
    

};
typedef Hrpc_SharedPtr<BindAdapter> BindAdapterPtr;
}
#endif
