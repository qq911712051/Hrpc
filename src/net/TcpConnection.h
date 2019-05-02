#ifndef HRPC_TCP_CONNECTION_H_
#define HRPC_TCP_CONNECTION_H_


#include <queue>
#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include <hrpc_socket.h>
#include <hrpc_lock.h>
#include <hrpc_ptr.h>
#include <hrpc_buffer.h>

#include <common.h>
#include <ConnectionBase.h>
namespace Hrpc
{

class NetThread;
class BindAdapter;



/**
 * 这个类 表示一条链接, 用来在网络线程和 业务线程中间  进行一些数据的传递
 * 
 * TcpConnection进行收发数据包时， 遵循的格式为
 *      |-----4bytes----|------data------------|---4bytes---|
 *   数据包大小（不包括头部长度） + 原始的数据   + 4字节的验错码
 */
class TcpConnection : public ConnectionBase, public std::enable_shared_from_this<TcpConnection> 
{
public:
    typedef std::queue<std::string> queue_type;
public:
    /**
     * @description: 初始化链接所在的socket
     * @param {type} 
     * @return: 
     */
    TcpConnection(int fd, BindAdapter* bind, int bufferlen = 1024);

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
     * @description: 发送响应包回网络线程
     *         在这里完成数据包的最外层封装操作
     * @param: msg 需要发送的数据
     * @return: 
     */
    void sendResponse(Hrpc_Buffer&& msg);

    /**
     * @description: 对于当前链接，进行心跳检测, 发送往业务线程
     * @param {type} 
     * @return: 
     */
    void sendHeartCheck();



    /**
     * @description: 关闭当前的链接, 给业务线程调用
     * @param {type} 
     * @return: 
     */
    void closeConnection();

    /**
     * @description: 将请求数据发往 业务线程处理
     * @param: buf 请求包原始数据
     * @return: 
     */
    void sendRequest(Hrpc_Buffer&& buf);

private:

    /**
     * @description: 禁止拷贝以及赋值
     * @param {type} 
     * @return: 
     */
    TcpConnection(const TcpConnection&) = delete;
    TcpConnection(TcpConnection&&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;
    TcpConnection& operator=(TcpConnection&&) = delete;
private:
    NetThread*          _netthread = {nullptr};        // 当前connection所属于的网络线程
    BindAdapter*        _bindAdapter = {nullptr};   // 当前connecton所属于的端口对象      
    
};

using TcpConnectionWeakPtr = std::weak_ptr<TcpConnection>;
// using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
}
#endif