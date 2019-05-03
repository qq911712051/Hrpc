#include <mutex>

#include <hrpc_time.h>

#include <ConnectionBase.h>

namespace Hrpc
{
// 设置最基本的验错码
std::string ConnectionBase::_validateCode = "####";              // 数据包验证码
size_t ConnectionBase::_maxPackageLength = 1024 * 1024 * 10;     // 数据包的最大长度为10M

ConnectionBase::ConnectionBase(int fd, size_t buflen)
{
    if (fd < 0)
    {
        throw Hrpc_Exception("connection para error， fd is negative");
    }

    if (buflen < 1024)
    {
        _bufferLen = 1024;
    }
    else
    {
        _bufferLen = buflen;
    }
    

    _tmpBuffer = new char[_bufferLen];
    // 初始化fd
    _sock.init(fd, true, AF_INET);
    _sock.setNonBlock();
    _sock.setNonDelay();

    _lastActivity = Hrpc_Time::getNowTimeMs();
}
void ConnectionBase::init(int fd, size_t buflen)
{
    if (fd < 0)
    {
        throw Hrpc_Exception("connection para error， fd is negative");
    }

    if (buflen < 1024)
    {
        _bufferLen = 1024;
    }
    else
    {
        _bufferLen = buflen;
    }
    

    _tmpBuffer = new char[_bufferLen];
    // 初始化fd
    _sock.init(fd, true, AF_INET);
    _sock.setNonBlock();
    _sock.setNonDelay();
}
ConnectionBase::~ConnectionBase()
{
    if (_tmpBuffer)
        delete [] _tmpBuffer;
}

int ConnectionBase::recvData()
{

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
                _recv_buffer.write(_tmpBuffer, cur);
                // 更新活动时间
                _lastActivity = Hrpc_Time::getNowTimeMs();
                return -1;
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
                _lastActivity = Hrpc_Time::getNowTimeMs();
                return -2;
            }
        }

        // 处理已经接受到的数据
        _recv_buffer.write(_tmpBuffer, cur);
        
    } while (!recvComplete);
    _lastActivity = Hrpc_Time::getNowTimeMs();
    return 0;
}

int ConnectionBase::sendData()
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
                // 异常
                return -3;
            }
        }

        // 正常发送完缓冲区所有数据
        _send_buffer.skipBytes(totalSize);
    }
    return 0;    
}
void ConnectionBase::pushData(Hrpc_Buffer&& buf)
{
    _send_buffer.pushData(std::move(buf));
}

bool ConnectionBase::extractPackage(Hrpc_Buffer& msg)
{
    if (_recv_buffer.size() > 4)
    {
        auto size = _recv_buffer.peekFrontInt32();
        if (size > 0)
        {
            if (size > _maxPackageLength)
            {
                std::cerr << "[ConnectionBase::extractPackage]: occur errro, recv package length too large = " << size << std::endl;        
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

                    msg = std::move(buffer);

                    return true;
                }
                else
                {
                    std::cerr << "[ConnectionBase::extractPackage]: occur errro, validateCode is invalid" << std::endl;        
                    // 数据异常， 运行错误处理机制
                    processError();
                    return true;
                }
                
            }
        }
        else
        {
            std::cerr << "[ConnectionBase::extractPackage]: occur errro package length" << std::endl;

            // 运行错误处理机制
            processError();
            return true;
        }
    }
    return false;
}

void ConnectionBase::processError(int type)
{
    // TODO: 这里先不处理type类型
    auto dest = _recv_buffer.find(_validateCode);
    if (dest != _recv_buffer.end())
    {
        _recv_buffer.skipTo(dest + _validateCode.size());
    }
    else
    {
        // 缓冲区中数据无法恢复， 对其进行清空处理
        _recv_buffer.clear();
        std::cerr << "[ConnecitonBase::processError]: can't process the error, clear the recv_buffer" << std::endl;
    }
    
}

void ConnectionBase::setUid(int uid)
{
    std::lock_guard<Hrpc_ThreadLock> sync(_lock);
    _uid = uid;
    _lock.notify();   
}
void ConnectionBase::waitForUidValid()
{
    if (_uid != -1)
        return;
    
    std::lock_guard<Hrpc_ThreadLock> sync(_lock);
    while (_uid == -1)
    {
        _lock.wait();
    }

}
}