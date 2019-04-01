#include <iostream>

#include <hrpc_lock.h>

#include <NetThreadGroup.h>
#include <BindAdapter.h>

namespace Hrpc
{

BindAdapter::BindAdapter(NetThreadGroup* group, const std::string& host, short port) : _threadGroup(group), _host(host), _port(port)
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


bool BindAdapter::accept(TcpConnectionPtr& ptr)
{
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
        ptr->setNetThread(dest);

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

/**
 * @description: 插入请求到队列
 * @param {type} 
 * @return: 
 */
void addRequest(RequestPtr req);

/**
 * @description: 阻塞获取新请求
 * @param: req 新请求   timeout  超时时间
 * @return: 是否超时
 */
bool getRequest(RequestPtr& req, int timeout);

/**
 * @description: 将需要进行心跳检测的链接插入到心跳检测队列队列 
 * @param: ptr  需要进行心跳检测的链接
 * @return: 
 */
void insertHeartConnection(TcpConnectionPtr ptr);

/**
 * @description: 获取所有的需要进行心跳检测的链接
 * @param: queue  心跳检测队列
 * @return: 
 */
void getAllHeartConnection(Hrpc_Queue<TcpConnectionPtr>& queue);

}