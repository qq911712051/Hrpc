#include <iostream>

#include <hrpc_lock.h>

#include <NetThreadGroup.h>
#include <BindAdapter.h>

namespace Hrpc
{

BindAdapter::BindAdapter(NetThreadGroup* group, const std::string& host, short port, int num, const std::string& name) 
    : _threadGroup(group), _host(host), _port(port), _threadNum(num), _objectName(name)
{

}


void BindAdapter::initialize()
{
    try
    {
        _listen.CreateSocket(SOCK_STREAM, AF_INET);
        _listen.setReuseAddr();     // 消除程序重启时time_wait状态的影响
        _listen.setNonBlock();      // 设置非阻塞
        _listen.setNonDelay();      // 关闭Nagle算法
        _listen.bind(_host, _port);
        _listen.listen(1024);

        // 新建业务数组
        for (int i = 0; i < _threadNum; i++)
        {
            _handles.push_back(Handle(new HandleThread(this)));
            // 初始化
            _handles[i]->intialize();
        }
    }
    catch (Hrpc_Exception& e)
    {
        std::cerr << e.what() << ", errcode = " << e.getErrCode() << std::endl;
        throw;
    }
    catch (const std::exception& ee)
    {
        std::cerr << "[BindAdapter::initialize]: " << ee.what() << std::endl;
        throw;
    }
    catch (...)
    {
        std::cerr << "[BindAdapter::initialize]: catch unknown exception" << std::endl;
        throw;
    }

}


bool BindAdapter::accept()
{
    ConnectionPtr ptr;
    try
    {
        Hrpc_Socket sock;
        sockaddr_in sa;
        socklen_t sock_len;
        bool res = _listen.accept(sock, (sockaddr*)&sa, sock_len);

        // 没有新的链接到来, 返回false
        if (!res)
            return false;
        // 设置这临时sock为 非owner
        sock.setOwner(false);

        // 初始化TcpConnection
        ptr.reset(new TcpConnection(sock.getFd(), this));
        
        // 设置TcpConnection的所属的网络线程
        NetThread* dest = _threadGroup->getNetThreadByRound();

        // 动态转换
        auto tcpConn = std::dynamic_pointer_cast<TcpConnection>(ptr);
        tcpConn->setNetThread(dest);
        // 添加此connection到对应的网络线程
        dest->addConnection(ptr);
    }
    catch (Hrpc_Exception& e)
    {
        std::cerr << "[BindAdapter::accept]: " << e.what() << ", errcode = " << e.getErrCode() << std::endl;
        return false;
    }
    catch (const std::exception& ee)
    {
        std::cerr << "[BindAdapter::accept]: " << ee.what() << std::endl;
        return false;
    }
    catch (...)
    {
        std::cerr << "[BindAdapter::accept]: catch unknown exception" << std::endl;
        return false;
    }
    return true;
}

void BindAdapter::addRequest(RequestPtr&& req)
{
    _request_queue.push(std::move(req));
}

std::queue<RequestPtr> BindAdapter::getAllRequest(int timeout)
{
    std::queue<RequestPtr> queue;
    
    // 交换
    _request_queue.swap(queue, timeout);
    return queue;
}

void BindAdapter::start()
{
    for (auto& t : _handles)
    {
        t->start(); // 业务线程启动
    }
}

BindAdapter::~BindAdapter()
{
    for (auto& t : _handles)
        t->terminate();
    
    for (auto& t : _handles)
        t->join();
}

bool BindAdapter::isRunning()
{
    for (auto& x : _handles)
    {
        if (!x->isRunning())
            return false;
    }
    return true;
}

void BindAdapter::addProtocol(std::unique_ptr<Hrpc_BaseProtocol>&& protocol)
{
    for (auto& handle : _handles)
    {
        handle->addHandleProtocol(std::move(protocol), Protocol::getName());
    }
}
void BindAdapter::setHeartProtocol(std::unique_ptr<Hrpc_BaseProtocol>&& protocol)
{
    for (auto& handle : _handles)
    {
        handle->setHeartProtocol(std::move(protocol));
    }
}

}