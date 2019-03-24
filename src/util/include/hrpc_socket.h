/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 封装了socket相关操作
 * @Date: 2019-03-22 18:25:57
 */

#ifndef HRPC_SOCKET_H_
#define HRPC_SOCKET_H_
#include <hrpc_exception.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace Hrpc
{

/**
 * @description: socket操作时抛出的异常类 
 */
class Hrpc_SocketException : public Hrpc_Exception
{
public:
    Hrpc_SocketException(const std::string& str) : Hrpc_Exception(str) {}
    Hrpc_SocketException(const std::string& str, int errCode) : Hrpc_Exception(str, errCode) {}
    ~Hrpc_SocketException() {}
};


/**
 * @description: 这是一个封装socket操作的一个类， 包括socket设置  
 */
class Hrpc_Socket
{
public:
    /**
     * @description: 默认构造 
     * @param {type} 
     * @return: 
     */
    Hrpc_Socket() : _fd(-1), _bOwner(false), _domain(0) {}


    /**
     * @description: 通过参数直接构造 
     * @param {type} 
     * @return: 
     */
    Hrpc_Socket(int fd, bool owner = false, int domain = AF_INET) : _fd(fd), _bOwner(owner), _domain(domain) {}

    /**
     * @description: 初始化socket
     * @param 
     * @return: 
     */
    void init(int fd, bool owner, int domain);

    /**
     * @description: 释放相关资源 
     * @param {type} 
     * @return: 
     */
    ~Hrpc_Socket();

    /**
     * @description: 初始化socket
     * @param： sockType  socket类型
     * @param： domain  域
     * @param： protocol 协议， 一般为0
     * @return: 
     */
    void CreateSocket(int sockType, int domain, int protocol = 0);

    /**
     * @description: 接受新的链接
     * @param: sock 新链接
     * @return: 返回是否接收成功
     */
    bool accept(Hrpc_Socket& sock, sockaddr* addr, socklen_t& len);

    /**
     * @description: 监听函数
     * @param: backlog 半连接和已完成连接但还没有accept的请求 之和  的最大值
     * @return: 
     */
    void listen(int backlog = 1024);

    /**
     * @description: 绑定端口和ip
     * @param: addr 目标地址，可能为域名，也可能为*.*.*.*格式
     * @param: port 目标端口
     * @return: 
     */
    void bind(const std::string& addr, short port);

    /**
     * @description: tcp三次握手动作
     * @param: addr 目标地址，可能为域名，也可能为*.*.*.*格式
     * @param: port 目标端口
     * @return: 
     */
    void connect(const std::string& addr, short port);

    /**
     * @description: 关闭连接，发送FIN
     *           减少描述符的引用计数， 当且仅当引用计数将至0时， 才会真正发送FIN以及回收资源 
     * @param {type} 
     * @return: 
     */
    void close();

    /**
     * @description: 关闭数据传输
     * @param: how 关闭的方向
     *          SHUT_WR , 关闭写通道， 此时，会发送FIN
     *          SHUT_RD,  关闭读， 不会发送FIN， 但是接收缓冲区中所有的数据被确认后丢弃
     *          SHUT_RDWR, 上面2种效果的叠加
     * @return: 
     */
    void shutdown(int how);

    /**
     * @description: tcp发送数据
     * @param: buf 数据缓冲区地址 
     * @param: len 数据长度 
     * @param: flags 标记 
     * @return: 返回值若大于0， 表示已经发送的字节数
     *          返回值若等于0， 表示对端关闭链接
     *          返回值若小于0， 非阻塞情况下EAGIN或者EWOULDBLOCK表示发送缓冲区已满，其余情况发生异常
     */
    int send(const char* buf, size_t len, int flags = 0);

    int sendto(const char* buf, size_t len, sockaddr* addr, socklen_t sock_len, int flags = 0);

    /**
     * @description: tcp接受数据
     * @param: buf 数据缓冲区地址 
     * @param: size 数据长度 
     * @param: flags 标记 
     * @return: 返回值若大于0， 表示已经接收的字节数
     *          返回值若等于0， 若无特殊情况，表示对端关闭链接(不考虑对端发送0字节的情况)
     *          返回值若小于0， 非阻塞情况下EAGIN或者EWOULDBLOCK表示数据接受完成
     */
    int recv(char* buf, size_t size, int flags = 0);

    int recvfrom(char* buf, size_t size, sockaddr* addr, socklen_t* pLen, int flags = 0);


    /**
     * @description: 获取tcp链接对端的地址信息 
     * @param: addr 对端的地址信息
     * @param: len  地址信息的长度
     * @return: 
     */
    void getPeerName(sockaddr* addr, socklen_t& len);
    
    /**
     * @description: 返回当前Socket的文件描述符
     * @param 
     * @return: 
     */
    int getFd() const {return _fd;}

    /**
     * @description: 设置当前描述符为非阻塞
     * @param: nonblock 是否非阻塞
     * @return: 
     */
    void setNonBlock(bool nonblock = true);

    /**
     * @description: 关闭Nagle算法，避免出现对向ack延迟确认时导致的延迟
     * @param: nondelay 是否关闭Nagle
     * @return: 
     */
    void setNonDelay(bool nondelay = true);

    /**
     * @description: 设置端口可以重复绑定，　在time_wait状态仍能进行ｂｉｎｄ
     * @param {type} 
     * @return: 
     */
    void setReuseAddr();

    /**
     * @description: 设置发送缓冲区大小
     * @param {type} 
     * @return: 
     */
    void setSendBufferSize(uint32_t size);

    /**
     * @description: 获取接受缓冲区的大小
     * @param {type} 
     * @return: 
     */
    uint32_t getSendBufferSize();


    /**
     * @description: 设置接受缓冲区 
     * @param: size 缓冲区大小。 单位字节
     *              客户端必须在connect之前设置
     *              服务端必须在listen之前设置
     * @return: 
     */
    void setRecvBufferSize(uint32_t size);

    /**
     * @description: 获取接收区缓冲区的大小
     * @param {type} 
     * @return: 
     */
    uint32_t getRecvBufferSize();
    
    
    /**
     * @description: 设置 close时等待缓冲器数据发送完毕的时间(TCP,STCP)
     * @param: seconds 等待时间，单位秒
     *          
     * @return: 
     */
    void setCloseWait(int seconds);

    /**
     * @description: 
     * @param {type} 
     * @return: 
     */
    void setOwner(bool owner = true);

    
private:
    Hrpc_Socket(const Hrpc_Socket&);
    Hrpc_Socket& operator=(const Hrpc_Socket&);

    /**
     * @description: 将字符串转化为地址
     * @param: addr 字符串地址 
     * @param: tIp  转化后的4字节网络地址
     * @return: 
     */
    bool parseAddr(const std::string& addr, in_addr_t& tIp);

    /**
     * @description: 设置socket参数
     * @param {type} 
     * @return: 
     */
    int setSocketOption(int level, int name, void* optval, socklen_t optlen);

    /**
     * @description: 获取socket的相关参数
     * @param {type} 
     * @return: 
     */
    int getSocketOption(int level, int name, void* optval, socklen_t* optlen);
private:
    int _fd;    // 所拥有的描述符
    bool _bOwner;   // 是否拥有此描述符
    int _domain;    // socket所属于的域
};
}
#endif