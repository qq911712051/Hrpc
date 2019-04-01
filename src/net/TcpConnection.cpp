#include <hrpc_time.h>

#include <BindAdapter.h>
#include <TcpConnection.h>
#include <NetThread.h>


namespace Hrpc
{
TcpConnection::TcpConnection(int fd, BindAdapter* bind)
    : _netthread(0), _bindAdapter(bind), _close(false)
{
    // 初始化其余变量
    _lastActivity = Hrpc_Time::getNowTime();

    // 初始化socket
    _sock.init(fd, true, AF_INET);
    _sock.setNonBlock();
    _sock.setNonDelay();

}

/**
 * @description: 释放相关资源
 * @param {type} 
 * @return: 
 */
~TcpConnection();

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

}