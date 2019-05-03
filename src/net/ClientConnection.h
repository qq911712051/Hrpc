#ifndef CLIENT_CONNECTION_H_
#define CLIENT_CONNECTION_H_
#include <memory>
#include <future>

#include <hrpc_buffer.h>
#include <hrpc_uid.h>

#include <ConnectionBase.h>
#include <HrpcProtocol.h>
namespace Hrpc
{

class ClientNetThread;
/**
 * 主要用作客户端的主动链接
 */
class ClientConnection : public ConnectionBase
{
    using Future_Data = std::unique_ptr<Hrpc_Buffer>;
    using WaitQueue = std::map<int, std::promise<Future_Data>>;
public:

    /**
     * @description: 构造一条新的链接
     * @param {type} 
     * @return: 
     */
    ClientConnection(ClientNetThread* net, int fd, int maxWaitNum, int bufferlen = 1024);

    /**
     * @description: 析构函数
     * @param
     * @return: 
     */
    ~ClientConnection() override;

    /**
     * @description: 发送信息到对端服务器 
     * @param {type} 
     * @return:  返回一个future， 等待结果返回
     */
    std::future<Future_Data> call_func_with_future(Hrpc_Buffer&& buf);

    /**
     * @description: 发送消息到对端服务器， 但是不会没有返回 
     * @param {type} 
     * @return: 
     */
    void call_func(Hrpc_Buffer&& buf);

    /**
     * @description: 客户端对于协议进行解析, 这里包含心跳协议解析
     * @param {type} 
     * @return: 
     */
    void parseHrpc(Hrpc_Buffer&& msg);
private:
    /**
     * @description: 添加网络包头部后发往网络线程
     * @param: msg  hrpc协议包数据
     * @return: 
     */
    void sendRequest(Hrpc_Buffer&& msg);

    /**
     * @description: 处理心跳协议
     * @param {type} 
     * @return: 
     */
    void processHeart(Hrpc_Buffer&& msg);
    
    /**
     * @description: 处理Hrpc协议
     * @param {type} 
     * @return: 
     */
    void processHrpc(Hrpc_Buffer&& msg);
private:
    ClientNetThread*    _thread;    // 网络线程

    UidGenarator        _seq;       // 用来生成seq序列号

    WaitQueue           _waitQueue; // 等待队列

    std::mutex          _lock;      // 保护等待队里的线程安全
    
};
using ClientConnectionPtr = std::shared_ptr<ClientConnection>;
}
#endif