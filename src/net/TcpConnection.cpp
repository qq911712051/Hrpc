#include <iostream>
#include <cassert>
#include <limits>
#include <thread>

#include <hrpc_time.h>

#include <BindAdapter.h>
#include <TcpConnection.h>
#include <NetThread.h>


namespace Hrpc
{



TcpConnection::TcpConnection(int fd, BindAdapter* bind, int bufferLen)
    : ConnectionBase(fd), _netthread(0), _bindAdapter(bind)
{
}

TcpConnection::~TcpConnection()
{
}

void TcpConnection::pushDataInSendBuffer(Hrpc_Buffer&& data)
{
    // _base.pushData(std::move(data));
    ConnectionBase::pushData(std::move(data));
}

void TcpConnection::sendResponse(Hrpc_Buffer&& msg)
{
    auto size = msg.size();
    
    if (size > 0)
    {
        // 每个消息包末尾另外添加4字节的验错码
        size_t totalSize = size + ConnectionBase::_validateCode.size(); 
        if (totalSize > INT32_MAX)
        {
            std::cerr << "[TcpConnection::sendResponse]: msg size too large" << std::endl;
            return;
        }
        // 压入首部长度
        msg.appendFrontInt32(totalSize);
        // 压入尾部验错码
        msg.write(ConnectionBase::_validateCode);

        // 封装成responseMessage
        auto resp = ResponsePtr(new ResponseMessage());
        // resp->_connection = shared_from_this();
        resp->_uid = ConnectionBase::getUid();
        resp->_type = ResponseMessage::HRPC_RESPONSE_NET;
        resp->_buffer = std::unique_ptr<Hrpc_Buffer>(new Hrpc_Buffer(std::move(msg)));

        // 将响应包 发送到网络线程的任务队列
        _netthread->insertResponseQueue(std::move(resp));
    }
    else
    {
        std::cerr << "[TcpConnection::sendResponse]: msg size error" << std::endl;
    }
    
}

void TcpConnection::sendHeartCheck()
{
    auto req = RequestPtr(new RequestMessage());
    
    // 设置请求类型为心跳检测
    req->_type = RequestMessage::HRPC_REQUEST_HEART;
    req->_connection = shared_from_this();
    req->_stamp = Hrpc_Time::getNowTimeMs();

    // 请求发送到 端口对应业务线程处理队列
    _bindAdapter->addRequest(std::move(req));

    // 设置标志位
    // startHeartChecking();
}


void TcpConnection::closeConnection()
{

    // 设置当前链接无效
    ConnectionBase::setClosed();   
    
    auto resp = ResponsePtr(new ResponseMessage());
    resp->_type = ResponseMessage::HRPC_RESPONSE_CLOSE;
    // resp->_connection = shared_from_this();
    resp->_uid  = ConnectionBase::getUid();
    
    // 将响应包发送到网络线程
    _netthread->insertResponseQueue(std::move(resp));
}

void TcpConnection::sendRequest(Hrpc_Buffer&& buf)
{
    auto req = RequestPtr(new RequestMessage());
    
    // 设置请求类型为心跳检测
    req->_type = RequestMessage::HRPC_REQUEST_FUNC;
    req->_connection = shared_from_this();
    req->_stamp = Hrpc_Time::getNowTimeMs();
    req->_buffer = std::unique_ptr<Hrpc_Buffer>(new Hrpc_Buffer(std::move(buf)));
    // 请求发送到 端口对应业务线程处理队列
    _bindAdapter->addRequest(std::move(req));
}

}