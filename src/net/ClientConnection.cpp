#include <ClientConnection.h>
#include <common.h>
#include <HeartProtocol.h>

namespace Hrpc
{
ClientConnection::ClientConnection(ClientNetThread* net, int fd, int maxWaitNum, int bufferlen) : ConnectionBase(fd, bufferlen), _thread(net)
{  
    _seq.init(maxWaitNum);
}

std::future<Future_Data> ClientConnection::call_func_with_future(Hrpc_Buffer&& buf)
{
    // 在这里进行数据的封装
    int seq = _seq.popUid();

    // 封装成一个Hrpc请求
    Hrpc_Buffer msg = HrpcProtocol::makeRequest(std::move(buf), seq, HrpcProtocol::HRPC_REQUEST);
    
    // 加入网络包头后发往网络线程
    sendRequest(std::move(msg));
    
    // 生成future
    std::promise<Future_Data> pro;
    auto fut = pro.get_future();    // 获得future
    
    // 压入等待队列
    {
        std::lock_guard<std::mutex> sync(_lock);
        _waitQueue[seq] = std::move(pro);
    }

    // 返回对应的future
    return fut;
}

void ClientConnection::call_func(Hrpc_Buffer&& buf)
{
    // 在这里进行数据的封装
    int seq = _seq.popUid();

    // 封装成一个Hrpc请求
    Hrpc_Buffer msg = HrpcProtocol::makeRequest(std::move(buf), seq, HrpcProtocol::HRPC_ONEWAY);
    
    // 加入网络包头后发往网络线程
    sendRequest(std::move(msg));
}

void ClientConnection::sendRequest(Hrpc_Buffer&& msg)
{
    int size = msg.size() + ConnectionBase::_validateCode.size();
    if (msg.beforeSize() >= 4)
    {
        // 压入头部大小
        msg.appendFrontInt32(size);
        // 压入尾部验错码
        msg.write(ConnectionBase::_validateCode);

        // 封装一个EpollServer消息处理包
        ResponsePtr resp = ResponsePtr(new ResponseMessage);
        resp->_buffer = std::unique_ptr<Hrpc_Buffer>(new Hrpc_Buffer(std::move(msg)));
        resp->_type = ResponseMessage::HRPC_RESPONSE_NET;
        resp->_uid = getUid();

        // 发往网络线程
        _thread->insertResponseQueue(std::move(resp));
    }
    else
    {
        Hrpc_Buffer res;
        res.appendInt32(size);
        res.pushData(std::move(msg));
        res.write(ConnectionBase::_validateCode);

        // 封装一个EpollServer消息处理包
        ResponsePtr resp = ResponsePtr(new ResponseMessage);
        resp->_buffer = std::unique_ptr<Hrpc_Buffer>(new Hrpc_Buffer(std::move(res)));
        resp->_type = ResponseMessage::HRPC_RESPONSE_NET;
        resp->_uid = getUid();

        // 发往网络线程
        _thread->insertResponseQueue(std::move(resp));
    }
}

void ClientConnection::parseHrpc(Hrpc_Buffer&& msg)
{
    // 客户端仅仅支持Hrpc
    size_t totalHead = 1 + HrpcProtocol::getName().size();

    if (msg.size() < totalHead)
    {
        // 协议出错
        std::cerr << "[ClientConnection::parseHrpc]: error hrpc package, contex=[" << msg.toByteString() << "]" << std::endl;
        return;
    }
    // 提取协议长度
    int length = msg.peekFrontInt8();
    msg.skipBytes(1);
    if (length <= 0)
    {
        std::cerr << "[ClientConnection::parseHrpc]: error hrpc package, protocol-length param = " << length << std::endl;
        return;
    }
    // 提取协议名称
    std::string protocolName = msg.get(0, length);
    msg.skipBytes(length);
    if (protocolName == "HRPC")
    {
        processHrpc(std::move(msg));
    }
    else if (protocolName == "HEART")
    {
        processHeart(std::move(msg));
    }
    else
    {
        std::cerr << "[ClientConnection::parseHrpc]: error hrpc package, protocol-name param = " << protocolName << std::endl;
        return;
    }
    
}

void ClientConnection::processHeart(Hrpc_Buffer&& msg)
{
    if (msg.size() != 1)
    {
        std::cerr << "[ClientConnection::processHeart]: error heart package, context = [" << msg.toByteString() << "]" << std::endl;
    }
    else
    {
        auto status = msg.peekFrontInt8();
        if (status == HeartProtocol::HEART_REQUEST)
        {
            // 进行心跳响应
            auto resp = HeartProtocol::makeResponse();

            // 发往网络线程
            sendRequest(std::move(resp));
        }
        else
        {
            std::cerr << "[ClientConnection::processHeart]: error heart package, status = " << status << std::endl;
        }
        
    }
    
}   

void ClientConnection::processHrpc(Hrpc_Buffer&& msg)
{
    // 提取序列号
    int seq = msg.peekFrontInt32();
    msg.skipBytes(4);
    
    // 提取状态
    int type = msg.peekFrontInt8();
    msg.skipBytes(1);
    if (type != HrpcProtocol::HRPC_RESPONSE)
    {
        std::cerr << "[ClientConnection::processHrpc]: error hrpc package, protocol-status param = " << type << std::endl;
        return;
    }

    bool succ = true;
    {
        std::lock_guard<std::mutex> sync(_lock);
        // 判断序列号合理性
        auto itr = _waitQueue.find(seq);
        if (itr != _waitQueue.end())
        {
            // 设置promise的值
            itr->second.set_value(Future_Data(new Hrpc_Buffer(std::move(msg))));
            // 从wait队列中删除
            _waitQueue.erase(itr);

            succ = true;
        }
        else
        {
            succ = false;
        }
    }
    if (succ)
    {
        std::cout << "[ClientConnection::processHrpc]: get Hrpc-response, seq = " << seq << std::endl;
    }
    else
    {
        std::cerr << "[ClientConnection::processHrpc]: error hrpc package format, protocol-sequence param = " << seq << ", not found wait future"<< std::endl;
    }
    
}
    
}