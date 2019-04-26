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
    : _netthread(0), _bindAdapter(bind), _close(false), _bufferLen(bufferLen)
{
    // 初始化其余变量
    _lastActivity = Hrpc_Time::getNowTimeMs();

    // 初始化socket
    _sock.init(fd, true, AF_INET);
    _sock.setNonBlock();
    _sock.setNonDelay();

    try
    {
        // 初始化临时缓冲区
        _tmpBuffer = new char[_bufferLen];
    }
    catch (const std::bad_alloc& e)
    {
        std::cerr << "[TcpConnection::TcpConnection]: alloc tmpBuffer error" << std::endl;
    }
}

TcpConnection::~TcpConnection()
{
    if (_tmpBuffer)
        delete [] _tmpBuffer;
}

void TcpConnection::pushDataInSendBuffer(Hrpc_Buffer&& data)
{
    _send_buffer.pushData(std::move(data));
}

void TcpConnection::sendResponse(Hrpc_Buffer&& msg)
{
    auto size = msg.size();
    
    if (size > 0)
    {
        // 每个消息包末尾另外添加4字节的验错码
        size_t totalSize = size + _validateCode.size(); 
        if (totalSize > INT32_MAX)
        {
            std::cerr << "[TcpConnection::sendResponse]: msg size too large" << std::endl;
            return;
        }
        // 压入首部长度
        msg.appendFrontInt32(totalSize);
        // 压入尾部验错码
        msg.write(_validateCode);

        // 封装成responseMessage
        auto resp = ResponsePtr(new ResponseMessage());
        resp->_connection = shared_from_this();
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

void TcpConnection::recvData()
{
    // 判断缓冲区是否分配成功
    if (!_tmpBuffer)
    {
        try
        {
            _tmpBuffer = new char[_bufferLen];
        }
        catch(const std::exception& e)
        {
            std::cerr << "[TcpConnection::recvData]: " << e.what() << std::endl;
            
            // 内存不足， 断开连接
            _netthread->closeConnection(_uid);
            
            return;
        }
        
    }

    bool recvComplete = false;
    do
    {
        // 开始接受数据
        size_t cur = 0;
        
        while (cur < _bufferLen)
        {
            int n = _sock.recv(_tmpBuffer + cur, _bufferLen - cur);
            if (n == 0)
            {
                // 对端关闭链接
                _netthread->closeConnection(_uid);
                std::cout << "[TcpConnection::recvData]: NetThread[" << std::this_thread::get_id()
                            << "], conneciton-id:" << _uid << " recv 0 bytes data, close connection" << std::endl;
                recvComplete = true;
                break;
            }
            else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            {
                // 数据接收完成,退出
                recvComplete = true;
                break;
            }
            else if (n > 0)
            {
                cur += n;
            }
            else
            {
                // 发生异常， 关闭链接
                _netthread->closeConnection(_uid);
                std::cerr << "[TcpConnection::recvData]: recv return -1 and errno = " << errno << std::endl;
                return;
            }
        }

        // 处理已经接受到的数据
        _recv_buffer.write(_tmpBuffer, cur);
        
    } while (!recvComplete);

    // 检测收到包的 完整性
    while (check());
}

bool TcpConnection::check()
{
    if (_recv_buffer.size() > 4)
    {
        auto size = _recv_buffer.peerFrontInt32();
        if (size > 0)
        {
            if (size > _maxPackageLength)
            {
                std::cerr << "[TcpConnection::check]: occur errro, recv package length too large = " << size << std::endl;        
                // 数据包长度出现异常
                processError();
                return true;
            }
            else if (_recv_buffer.size() >= 4 + size)
            {
                auto validateCode = _recv_buffer.get(size, 4);
                if (validateCode == _validateCode)
                {
                    // 数据包正常
                    auto buffer = _recv_buffer.getToBuffer(4, size - 4);
                    // 更新数据接受缓冲区
                    _recv_buffer.skipBytes(4 + size);

                    // 发往业务线程
                    sendRequest(std::move(buffer));

                    return true;
                }
                else
                {
                    std::cerr << "[TcpConnection::check]: occur errro, validateCode is invalid" << std::endl;        
                    // 数据异常， 运行错误处理机制
                    processError();
                    return true;
                }
                
            }
        }
        else
        {
            std::cerr << "[TcpConnection::check]: occur errro package length" << std::endl;

            // 运行错误处理机制
            processError();
            return true;
        }
    }
    return false;
}

void TcpConnection::processError(int type)
{
    // TODO: 这里先不处理type类型
    auto dest = _recv_buffer.find(_validateCode);
    if (dest != _recv_buffer.end())
    {
        _recv_buffer.skipTo(dest);
    }
    else
    {
        // 缓冲区中数据无法恢复， 对其进行清空处理
        _recv_buffer.clear();
        std::cerr << "[TcpConnection::processError]: can't process the error, clear the recv_buffer" << std::endl;
    }
    
}


int TcpConnection::sendData()
{
    if (_send_buffer.size() > 0)
    {
        size_t hasSend = 0;
        size_t totalSize = _send_buffer.size();
        while (hasSend < totalSize)
        {
            int n = _sock.send(_send_buffer.begin() + hasSend, totalSize - hasSend);

            if (n == 0)
            {
                // 对端关闭链接
                _netthread->closeConnection(_uid);
                std::cout << "[TcpConnection::sendData]: NetThread[" << std::this_thread::get_id()
                            << "], conneciton-id:" << _uid << " send 0 bytes data, close connection" << std::endl;
                
                return -1;
            }
            else if (n > 0)
            {
                // 发送成功
                hasSend += n;
            }
            else if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            {
                // 发送缓冲区满了
                // 修改发送缓冲区游标
                _send_buffer.skipBytes(hasSend);
                return -2;
            }
            else
            {
                // 异常情况
                std::cerr << "[TcpConnection::sendData]: occur error, send return -1, and errno = " << errno << std::endl;
                _netthread->closeConnection(_uid);
                return -3;
            }
        }

        // 正常发送完缓冲区所有数据
        _send_buffer.skipBytes(totalSize);
    }
    return 0;    
}


void TcpConnection::closeConnection()
{
    
    auto resp = ResponsePtr(new ResponseMessage());
    resp->_type = ResponseMessage::HRPC_RESPONSE_CLOSE;
    resp->_connection = shared_from_this();
    
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