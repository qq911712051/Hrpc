/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 一条包含缓冲区的socket链接 
 * @Date: 2019-04-30 21:28:06
 */
#ifndef CONNECTION_BASE_H_
#define CONNECTION_BASE_H_
#include <iostream>
#include <memory>

#include <hrpc_socket.h>
#include <hrpc_buffer.h>
#include <hrpc_lock.h>
#include <hrpc_exception.h>
namespace Hrpc
{

class ConnectionBase
{
public:
    ConnectionBase() = default;

    virtual ~ConnectionBase();
    
    ConnectionBase(int fd, size_t buflen = 1024);

    /**
     * @description: 初始化当前链接 
     * @param {type} 
     * @return: 
     */
    void init(int fd, size_t buflen = 1024);
    /**
     * @description:  从此链接接收数据
     * @param {type} 
     * @return: 返回值表示状态
     *          若为0， 表示正常接受
     *          若为-1， 表示对端关闭链接
     *          若为-2， 异常情况，关闭链接
     */
    int recvData();

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
     * @description: 提取完整的网络包
     * @param {type} 
     * @return: 
     */
    bool extractPackage(Hrpc_Buffer& msg);

    /**
     * @description: 将数据压到 待发送缓冲区
     * @param {type} 
     * @return: 
     */
    void pushData(Hrpc_Buffer&& buf);

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
     * @description: 获取socket的fd 
     * @param {type} 
     * @return: 
     */
    int getFd() const {return _sock.getFd();}

    /**
     * @description: 获取send_buffer的大小
     * @param {type} 
     * @return: 
     */
    size_t sendBufferSize() const {return _send_buffer.size();}
    /**
     * @description: 获取recv_buffer的大小
     * @param {type} 
     * @return: 
     */
    size_t recvBufferSize() const {return _recv_buffer.size();}

    /**
     * @description: 设置当前connection的uid
     * @param {type} 
     * @return: 
     */
    void setUid(int uid);

    /**
     * @description: 等待此链接uid生效 
     * @param {type} 
     * @return: 
     */
    void waitForUidValid();

    /**
     * @description: 获取当前connection的uid
     * @param {type} 
     * @return: 
     */
    int getUid() const {return _uid;}

    /**
     * @description: 设置此链接为无效链接 
     * @param {type} 
     * @return: 
     */
    void setClosed() {_close = true;}

    /**
     * @description: 此条链接是否有效
     * @param {type} 
     * @return: 返回是否有效
     */
    bool isValid() const {return _close;}

public:
    static std::string  _validateCode;          // 数据包结尾的验错码
    static size_t       _maxPackageLength;      // 数据包的最大长度
private:
    /**
     * @description: 当网络包出现错误时， 处理
     * @param: 错误类型
     * @return: 
     */
    void processError(int type = 0);
private:
    Hrpc_Socket     _sock;      // 当前链接拥有的链接
    Hrpc_Buffer     _recv_buffer;   // 接受缓冲区
    Hrpc_Buffer     _send_buffer;   // 发送缓冲区

    size_t          _lastActivity = {0};  // 最后的活动时间
    bool            _close = {false};         // 当前链接是否已经失效
    int             _uid = {-1};           // 标识此链接在某个网络线程中的uid
    Hrpc_ThreadLock _lock;                  // 用于同步uid, uid更新时， 表示此链接已经别 epoll监管

    char*           _tmpBuffer = {nullptr}; // 作为临时缓冲区使用， 避免频繁的内存分配
    int             _bufferLen = {1024}; // 临时缓冲区的长度
};

using ConnectionPtr = std::shared_ptr<ConnectionBase>;  // 指向链接基类的智能指针
}
#endif