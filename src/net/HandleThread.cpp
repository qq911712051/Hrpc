#include <iostream>

#include <HandleThread.h>
#include <BindAdapter.h>
#include <common.h>
#include <HeartProtocol.h>

namespace Hrpc
{
HandleThread::HandleThread(BindAdapter* bind) : _bindAdapter(bind)
{
    
}

void HandleThread::setHeartProtocol(Protocol&& proto)
{
    _protoFactory["HEART"] = std::move(proto);
}

void HandleThread::intialize()
{
    // 添加默认的心跳协议到协议工厂
    _protoFactory["HEART"] = Protocol(new HeartProtocol());
}
void HandleThread::run()
{

    while (!_terminate)
    {
        // 获取请求, 阻塞等待
        std::queue<RequestPtr> rq = _bindAdapter->getAllRequest();

        while (!rq.empty())
        {
            auto req = std::move(rq.front());
            rq.pop();

            // 对于不同的请求类型进行处理
            switch (req->_type)
            {
                case RequestMessage::HRPC_REQUEST_HEART:
                {
                    // 由网络线程发送过来。通过业务线程发送出去
                    processHeart(req);
                    break;
                }
                case RequestMessage::HRPC_REQUEST_FUNC:
                {
                    // 如果是正常的请求
                    processFunc(req);
                    break;
                }
                default:
                {
                    std::cerr << "[HandleThread::run]: unknown request type = " << req->_type << std::endl;
                    break;
                }
            }
        }
    }

    std::cerr << "[HandleThread::run]: object[" << _bindAdapter->getObjectName() << "], handle thread end" << std::endl;
}

void HandleThread::processHeart(const RequestPtr& req)
{
    auto conn = req->_connection.lock();
    if (conn)
    {
        // 发送一个标准的HEART协议， 协议体为空
        auto buf = HeartProtocol::doRequest();

        // 发送请求到网络线程
        conn->sendResponse(std::move(buf));

        // 开启标记
        // conn->startHeartChecking();
    }
    else
    {
        std::cerr << "[HandleThread::processHeart]: requestMessage is invalid, the connection is invalid" << std::endl;
    }
    
}

void HandleThread::processFunc(const RequestPtr& req)
{
    auto conn = req->_connection.lock();
    if (conn)
    {
        Hrpc_Buffer msg = std::move(*req->_buffer);
        std::string protoName;
        if (checkRequest(msg, protoName))
        {
            // 跳过头部字节
            msg.skipBytes(1 + protoName.size());
            auto response = distribute(std::move(msg), protoName);
            
            // 判断返回值是否为空
            if (response.size() != 0)
            {
                // 需要进行回包
                conn->sendResponse(std::move(response));
            }
        }
        else
        {
            std::cerr << "[HandleThread::processFunc]: request package data error [" << msg.toByteString() << "]" << std::endl;
        }
        
    }
    else
    {
        std::cerr << "[HandleThread::processFunc]: requestMessage is invalid, the connection is invalid" << std::endl;
    }
}


Hrpc_Buffer HandleThread::distribute(Hrpc_Buffer&& buf, const std::string& name)
{
    auto itr = _protoFactory.find(name);
    if (itr != _protoFactory.end())
    {
        // 对数据包进行解析
        return itr->second->parse(std::move(buf));
    }
    else
    {
        std::cerr << "[HandleThread::distribute]: unknown protocol = " << name << std::endl;
    }
    return Hrpc_Buffer();
}

HandleThread::~HandleThread()
{
    // 等待线程结束
    terminate();
}

void HandleThread::addHandleProtocol(Protocol&& proto, const std::string protoName)
{
    // 寻找协议是否存在
    auto itr = _protoFactory.find(protoName);
    if (itr != _protoFactory.end())
    {
        _protoFactory[protoName] = std::move(proto);
    }
    else
    {
        throw Hrpc_Exception("[HandleThread::addHandleProtocol]: the protocol [" + protoName + "] has exist");
    }
    

}

bool HandleThread::checkRequest(const Hrpc_Buffer& buf, std::string& protoName)
{
    if (buf.size() < 1)
        return false;
    
    auto length = buf.peekFrontInt8();
    if (length > 0)
    {
        protoName = buf.get(1, length);
        if (protoName == "")
            return false;
    }
    else
    {
        return false;
    }
    return true;
}

}