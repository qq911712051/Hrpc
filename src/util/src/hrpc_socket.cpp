/*
* @author: blackstar0412
* @github: github.com/qq911712051
* @description: 对于Hrpc_Socket的实现
* @Date: 2019-03-22 18:26:05
*/
#include <hrpc_socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <hrpc_time.h>
namespace Hrpc
{

void Hrpc_Socket::init(int fd, bool owner, int domain)
{
    // 当前socket已经存在
    if (_fd >= 0)
    {
        close();
    }
    
    _fd = fd;
    _bOwner = owner;
    _domain = domain;
}


Hrpc_Socket::~Hrpc_Socket()
{
    if (_fd >=0)
    {
        close();   
    }
}

void Hrpc_Socket::CreateSocket(int sockType, int domain, int protocol)
{
    if (_fd < 0)
    {
        int tFd = ::socket(domain, sockType, protocol);
        if (tFd < 0)
        {
            throw Hrpc_SocketException("create socket error, errno saved", errno);
        }     
        _fd = tFd;
        _domain = domain;
        _bOwner = true;
    }
    else
    {
        throw Hrpc_SocketException("create socket error, socket fd exist");
    }
    
}

void Hrpc_Socket::setNonBlock(bool nonblock)
{
    if (_fd < 0)
        throw Hrpc_SocketException("setNonBlock error, socket fd is invalid");
    
    // 获取设置
    int flag = fcntl(_fd, F_GETFL, 0);    
    if (nonblock)
        flag |= SOCK_NONBLOCK;
    else
        flag = ~((~flag) | SOCK_NONBLOCK);
    fcntl(_fd, F_SETFL, flag);
}


void Hrpc_Socket::setNonDelay(bool nondelay)
{
    if (_fd < 0)
        throw Hrpc_SocketException("[Hrpc_Socket::setNonDelay]: socket fd is invalid");

    int flag = 0;
    if (nondelay)   
        flag = 1;
    // setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, &flag, socklen_t(sizeof(int)));
    if (setSocketOption(IPPROTO_TCP, TCP_NODELAY, &flag, socklen_t(sizeof(int))) < 0)
        throw Hrpc_SocketException("[Hrpc_Socket::setNonDelay]: setSocketOption error, errno saved", errno);
       
}

void Hrpc_Socket::setCloseWait(int seconds)
{
    if (_fd < 0)
        throw Hrpc_SocketException("[Hrpc_Socket::setCloseWait]: socket fd is invalid");

    if (seconds < 0)
        seconds = 0;
    linger lg;
    lg.l_onoff = 1;
    lg.l_linger = seconds;

    if (setSocketOption(SOL_SOCKET, SO_LINGER, &lg, socklen_t(sizeof(linger))) < 0)
        throw Hrpc_SocketException("[Hrpc_Socket::setCloseWait]: setSocketOption error, errno saved", errno);

}


void Hrpc_Socket::setOwner(bool owner)
{
    _bOwner = owner;

}

void Hrpc_Socket::close()
{
    // 无需判断fd是否有效
    if (_bOwner)
    {
        ::close(_fd);
        _fd = -1;
    }
}




bool Hrpc_Socket::accept(Hrpc_Socket& sock, sockaddr* sa, socklen_t& sl)
{
    if (_fd >= 0)
    {
        int ifd = -1;
        // 若是信号导致的中断， 继续请求
        while ((ifd = ::accept(_fd, sa, &sl)) < 0 && errno == EINTR);

        if (ifd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return false;
            else
            {
                throw Hrpc_SocketException("[Hrpc_Socket::accept]: accpet error, errno saved", errno);
            }
        }
        else
        {
            sock.init(ifd, true, _domain);
        }
        return true;
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::accept]: _fd is invalid");
    }
    
}


    // if (_fd >= 0)
    // {

    // }
    // else
    // {
    //     throw Hrpc_SocketException("[Hrpc_Socket::listen]: _fd is invalid");
    // }
void Hrpc_Socket::listen(int backlog)
{

    if (_fd >= 0)
    {
        if (backlog < 0)
        {
            throw Hrpc_SocketException("[Hrpc_Socket::listen]: backlog paramer is incorrect");
        }
        
        if (::listen(_fd, backlog) < 0)
        {
            throw Hrpc_SocketException("[Hrpc_Socket::listen]: listen error, errno saved");
        }
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::listen]: _fd is invalid");
    }
    
}

void Hrpc_Socket::setReuseAddr()
{
    if (_fd >= 0)
    {
        int flag = 1;
        // setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &flag, socklen_t(sizeof(int)));
        if (setSocketOption(SOL_SOCKET, SO_REUSEADDR, &flag, socklen_t(sizeof(int))) < 0)
            throw Hrpc_SocketException("[Hrpc_Socket::setReuseAddr]: setSocketOption error, errno saved", errno);
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::setReuseAddr]: _fd is invalid");
    }
}

void Hrpc_Socket::bind(const std::string& addr, short port)
{
    if (_fd >= 0)
    {
        in_addr_t tIp = 0;
        // 得到目标地址
        if (parseAddr(addr, tIp))
        {

            sockaddr_in sin;
            sin.sin_family = _domain;
            sin.sin_port = htons(port);
            sin.sin_addr.s_addr = tIp;

            if(::bind(_fd, (sockaddr*)&sin, sizeof(sockaddr)) < 0)
            {
                throw Hrpc_SocketException("[Hrpc_Socket::bind]: bind error, errno saved");
            }
        }
        else
        {
            throw Hrpc_SocketException("[Hrpc_Socket::bind]: destination address is invalid");
        }
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::bind]: _fd is invalid");
    }
}

bool Hrpc_Socket::parseAddr(const std::string& addr, in_addr_t& tIp)
{
    int res = inet_pton(AF_INET, addr.c_str(), &tIp);

    // 成功转化
    if (res > 0)
    {
        return true;
    }
    else if (res == 0)
    {
        // 可能为域名
        hostent* pHostent = ::gethostbyname(addr.c_str());
        if (pHostent && (*pHostent->h_addr_list))
        {
            // 设置地址
            tIp = *(uint32_t*)(*pHostent->h_addr_list);
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    return true;
}


void Hrpc_Socket::connect(const std::string& addr, short port)
{
    if (_fd >= 0)
    {
        in_addr_t tIp = 0;
        if (parseAddr(addr, tIp))
        {
            sockaddr_in sa;
            sa.sin_family = AF_INET;
            sa.sin_addr.s_addr = tIp;
            sa.sin_port = htons(port);

            // connect 发生EINTR时不能重启调用
            // int res = -1;
            // while ((res = ::connect(_fd, (sockaddr*)&sa, sizeof(sockaddr_in))) < 0 && errno == EINTR);
            int res = ::connect(_fd, (sockaddr*)&sa, sizeof(sockaddr_in));

            if (res < 0)
            {
                if (errno == EINPROGRESS || errno == EINTR)
                {
                    int tmpRes = -1;
                    char dummy[1];

                    // 直到链接建立 10ms等待
                    size_t t1 = Hrpc_Time::getNowTimeMs();
                    while ((tmpRes = ::read(_fd, dummy, 0))!= 0)
                    {
                        if (Hrpc_Time::getNowTimeMs() - t1 > 10)
                            break;
                        else
                        {
                            // 睡眠1ms
                            ::usleep(1000);
                        }
                        
                    }
                    if (tmpRes != 0)
                    {
                        // 发生异常
                        throw Hrpc_SocketException("[Hrpc_Socket::connect]: connect error");
                    }
                }
                else
                {
                    throw Hrpc_SocketException("[Hrpc_Socket::connect]: connect error, errno saved", errno);
                }                
            }
        }
        else
        {
            throw Hrpc_SocketException("[Hrpc_Socket::connect]: invalid address");
        }
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::connect]: _fd is invalid");
    }
}


void Hrpc_Socket::shutdown(int how)
{
    if (_fd >= 0)
    {
        ::shutdown(_fd, how);
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::shutdown]: _fd is invalid");
    }
}

int Hrpc_Socket::send(const char* buf, size_t len, int flags)
{
    if (_fd >= 0)
    {
        int ret = -1;
        while ((ret = ::send(_fd, buf, len, flags)) < 0 && errno == EINTR);
        return ret;
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::send]: _fd is invalid");
    }
    return 0;
}

int Hrpc_Socket::sendto(const char* buf, size_t len, sockaddr* addr, socklen_t sock_len, int flags)
{
    if (_fd >= 0)
    {

        int ret = -1;
        while ((ret = ::sendto(_fd, buf, len, flags, addr, len)) < 0 && errno == EINTR);
        return ret;
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::sendto]: _fd is invalid");
    }
    return 0;
}

int Hrpc_Socket::recv(char* buf, size_t size, int flags)
{
    if (_fd >= 0)
    {
        int ret = -1;
        while ((ret = ::recv(_fd, buf, size, flags)) < 0 && errno == EINTR);
        return ret;
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::recv]: _fd is invalid");
    }
    return 0;
}

int Hrpc_Socket::recvfrom(char* buf, size_t size, sockaddr* addr, socklen_t* pLen, int flags)
{
    if (_fd >= 0)
    {
        int ret = -1;
        while ((ret = ::recvfrom(_fd, buf, size, flags, addr, pLen)) < 0 && errno == EINTR);
        return ret;
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::recvfrom]: _fd is invalid");
    }
    return 0;
}


void Hrpc_Socket::getPeerName(sockaddr* addr, socklen_t& len)
{
    if (_fd >= 0)
    {
        if (::getpeername(_fd, addr, &len) < 0)
        {
            throw Hrpc_SocketException("[Hrpc_Socket::getPeerName]: getpeername error", errno);
        }
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::getPeerName]: _fd is invalid");
    }
}


int Hrpc_Socket::setSocketOption(int level, int name, void* optval, socklen_t optlen)
{
    return ::setsockopt(_fd, level, name, optval, optlen);
}

int Hrpc_Socket::getSocketOption(int level, int name, void* optval, socklen_t* optlen)
{
    return ::getsockopt(_fd, level, name, optval, optlen);
}

void Hrpc_Socket::setSendBufferSize(uint32_t size)
{
    if (_fd >= 0)
    {
        if (setSocketOption(SOL_SOCKET, SO_SNDBUF, &size, sizeof(socklen_t)) < 0)
            throw Hrpc_SocketException("[Hrpc_Socket::setSendBufferSize]: setSocketOption error, errno saved", errno);
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::setSendBufferSize]: _fd is invalid");
    }
}

void Hrpc_Socket::setRecvBufferSize(uint32_t size)
{
    if (_fd >= 0)
    {
        if (setSocketOption(SOL_SOCKET, SO_RCVBUF, &size, sizeof(socklen_t)) < 0)
            throw Hrpc_SocketException("[Hrpc_Socket::setRecvBufferSize]: setSocketOption error, errno saved", errno);
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::setRecvBufferSize]: _fd is invalid");
    }
}

uint32_t Hrpc_Socket::getSendBufferSize()
{
    if (_fd >= 0)
    {
        uint32_t size = 0;
        socklen_t len = sizeof(uint32_t);
        if (getSocketOption(SOL_SOCKET, SO_SNDBUF, &size, &len) < 0)
            throw Hrpc_SocketException("[Hrpc_Socket::getSendBufferSize]: getSocketOption error, errno saved", errno);
        return size;
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::getSendBufferSize]: _fd is invalid");
    }
    return 0;
}

uint32_t Hrpc_Socket::getRecvBufferSize()
{
    if (_fd >= 0)
    {
        uint32_t size = 0;
        socklen_t len = sizeof(uint32_t);
        if (getSocketOption(SOL_SOCKET, SO_RCVBUF, &size, &len) < 0)
            throw Hrpc_SocketException("[Hrpc_Socket::getRecvBufferSize]: getSocketOption error, errno saved", errno);
        return size;
    }
    else
    {
        throw Hrpc_SocketException("[Hrpc_Socket::getRecvBufferSize]: _fd is invalid");
    }
    return 0;
}

}