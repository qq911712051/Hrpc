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
class TcpConnection : public std::enable_shared_from_this<TcpConnection>
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
     * @description: 设置当前链接最后的活动时间
     * @param {type} 
     * @return: 
     */
    void setLastActivityTime(size_t time) {_lastActivity = time;}

    /**
     * @description: 获取当前链接最后的活动时间
     * @param {type} 
     * @return: 
     */
    size_t getLastActivityTime() const {return _lastActivity;}

    /**
     * @description: 设置当前connection的uid
     * @param {type} 
     * @return: 
     */
    void setUid(int uid) {_uid = uid;}

    /**
     * @description: 获取当前connection的uid
     * @param {type} 
     * @return: 
     */
    int getUid() const {return _uid;}

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
     * @description: 从此链接接受数据,若接收到完整数据包， 即发送给业务线程
     * @param {type} 
     * @return: 
     */
    void recvData();

    /**
     * @description: 将待发送缓冲区的所有数据发往 此链接对应的socket 
     * @param {type} 
     * @return: 返回当前数据发送的状态
     *         若为0， 表示缓冲区全部发送完成
     *         若为-1， 表示对端关闭链接
     *         若为-2， 表示当前socket的发送缓冲区已满, 部分数据还未发送完成
     *         若为-3. 异常情况，并且关闭链接
     */
    int sendData();

    /**
     * @description: 将未发送完的数据压入 待发送缓冲区
     * @param: data 待发送的数据， 此变量为右值
     * @return: 
     */
    void pushDataInSendBuffer(Hrpc_Buffer&& data);

    /**
     * @description: 上一次发送数据时 是否全部发送成功 
     * @param
     * @return: 
     */
    bool isLastSendComplete() const {return _send_buffer.size() == 0;}

    /**
     * @description: 是否正在等待心跳响应包 返回
     * @param {type} 
     * @return: 
     */
    bool isHeartChecking() const {return _heartChecking;}

    /**
     * @description: 心跳检测结束
     * @param {type} 
     * @return: 
     */
    void setHeartCheckComplete() {_heartChecking = false;}

    /**
     * @description: 对当前链接进行心跳检测
     * @param {type} 
     * @return: 
     */
    void startHeartChecking() {_heartChecking = true;}

    /**
     * @description: 获取当前链接的fd
     * @param {type} 
     * @return: 
     */
    int getFd() const 
    {
        return _sock.getFd();
    }

    /**
     * @description: 关闭当前的链接, 给业务线程调用
     * @param {type} 
     * @return: 
     */
    void closeConnection();
private:

    /**
     * @description: 将请求数据发往 业务线程处理
     * @param: buf 请求包原始数据
     * @return: 
     */
    void sendRequest(Hrpc_Buffer&& buf);
    /**
     * @description: 检测当前是否收到完整的数据包
     *      如果有完整数据包， 则将其发送到业务线程的处理队列 
     * @param {type} 
     * @return: 是否检测一个完整数据包
     */    
    bool check();

    /**
     * @description: 对于数据包处理时发生的错误 进行处理
     * @param: type 错误的类型 
     * @return: 
     */
    void processError(int type = 0);
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

    Hrpc_Buffer         _recv_buffer;   // 接受缓冲区, 只存在一个网络线程访问，线程安全

    Hrpc_Buffer         _send_buffer;   // 保存未发送完成的数据, 只能存在网络线程访问， 因此线程安全

    Hrpc_Socket         _sock;          // 表示当前链接
    
    bool                _close = {false};         // 当前链接是否已经失效

    int                 _uid = {-1};           // 标识此链接在某个网络线程中的uid

    size_t              _lastActivity = {0};  // 当前链接的最后一次活动时间
    
    bool                _heartChecking = {false}; // 当前链接是否已经发送心跳检测包

    std::mutex          _lock;         

    char*               _tmpBuffer = {nullptr}; // 作为临时缓冲区使用， 避免频繁的内存分配
    int                 _bufferLen = {1024}; // 临时缓冲区的长度
    
    static std::string  _validateCode;          // 数据包结尾的验错码
    static size_t       _maxPackageLength;      // 数据包的最大长度
};

using TcpConnectionWeakPtr = std::weak_ptr<TcpConnection>;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
}
#endif