#ifndef HRPC_TCP_CONNECTION_H_
#define HRPC_TCP_CONNECTION_H_


#include <queue>
#include <string>
#include <memory>
#include <mutex>

#include <hrpc_socket.h>
#include <hrpc_lock.h>
#include <hrpc_ptr.h>
#include <hrpc_buffer.h>

#include <common.h>
namespace Hrpc
{

class NetThread;
class BindAdapter;

/**
 * 业务线程处理的请求包结构体
 */
struct RequestMessage
{
public:
    TcpConnectionPtr        _connection;    // 标识消息对一个的链接
    Hrpc_Buffer             _buffer;        // 请求包的内容
    size_t                  _stamp;         // 接收请求的时间戳  
    int                     _type;          // 类别。正常的客户端请求， 网络线程发送的心跳包等等
};

struct ResponseMessage
{
public:
    Hrpc_Buffer             _buffer;        // 响应包内容
    int                     _type;          // 响应类型
};

using RequestPtr = std::unique_ptr<RequestMessage>;

using ResponsePtr = std::unique_ptr<ResponseMessage>;


class TcpConnection
{
public:
    typedef std::queue<std::string> queue_type;
public:
    /**
     * @description: 初始化链接所在的socket
     * @param {type} 
     * @return: 
     */
    TcpConnection(int fd, BindAdapter* bind);

    /**
     * @description: 释放相关资源
     * @param {type} 
     * @return: 
     */
    ~TcpConnection();

    /**
     * @description: 设置当前connection所属于的网络网络线程
     * @param {type} 
     * @return: 
     */
    void setNetThread(NetThread* net) {_netthread = net;}

    /**
     * @description: 发送消息往网络线程
     * @param {type} 
     * @return: 
     */
    void sendResponse(const std::string& msg);

    /**
     * @description: 对于当前链接，进行心跳检测
     * @param {type} 
     * @return: 
     */
    void sendHeartCheck();

    /**
     * @description: 从此链接接受数据,若接收到完整数据包， 即发送给业务线程
     * @param {type} 
     * @return: 
     */
    void recvData();

    /**
     * @description: 获取当前链接的socket
     * @param: sock 获取的socket
     * @return: 
     */
    void getSocket(Hrpc_Socket& sock);
private:

    /**
     * @description: 禁止拷贝以及赋值
     * @param {type} 
     * @return: 
     */
    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;


private:
    NetThread*          _netthread;        // 当前connection所属于的网络线程
    BindAdapter*        _bindAdapter;   // 当前connecton所属于的端口对象      

    Hrpc_Buffer         _recv_buffer;   // 接受缓冲区, 只存在一个网络线程访问，线程安全

    Hrpc_Buffer         _send_buffer;   // 发送缓冲区， 非线程安全， 需要加锁保护

    Hrpc_Socket         _sock;          // 表示当前链接
    
    bool                _close;         // 当前链接是否已经失效

    size_t              _lastActivity;  // 当前链接的最后一次活动时间

    std::mutex          _lock;          // 保护发送缓冲区
 
};
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TcpConnectionWeakPtr = std::weak_ptr<TcpConnection>;
}
#endif