#include <iostream>
#include <thread>

#include <sys/epoll.h>

#include <hrpc_common.h>

#include <NetThread.h>
using namespace std::placeholders;
namespace Hrpc
{

// NetThread::NetThread(NetThreadGroup* ptr, int maxConn, int wait, int heartTime) : _threadGroup(ptr), _terminate(false), _Max_connections(maxConn), _waitTime(wait), _heartTime(heartTime)
NetThread::NetThread(NetThreadGroup* ptr, const Hrpc_Config& config) : _threadGroup(ptr), _terminate(false)
{
    std::string data = config.getString("/hrpc/server/NetThread/MaxConnection");
    int maxConn = Hrpc_Common::strto<int>(data);
    if (maxConn <= 0)
    {
        maxConn = 1024;     // 若没有定义， 则默认1024
    }
    
    std::cout << "NetThread [" << std::this_thread::get_id() << "] set MaxConnection param [" << maxConn << "]" << std::endl;


    // epoll时间
    data = config.getString("/hrpc/server/NetThread/EpollWaitTime");

    int epollTime = Hrpc_Common::strto<int>(data);
    if (epollTime <= 0)
    {
        epollTime = 10;
    }

    std::cout << "NetThread [" << std::this_thread::get_id() << "] set EpollWaitTime param [" << epollTime << "]" << std::endl;
    // 心跳协议时间
    data = config.getString("/hrpc/server/NetThread/HeartTime");

    int heartTime = Hrpc_Common::strto<int>(data);
    if (heartTime <= 1000)
    {
        heartTime = 1000;
    }
    _heartTime = heartTime;
    std::cout << "NetThread [" << std::this_thread::get_id() << "] set HeartTime param [" << heartTime << "]" << std::endl;

    // 初始化EpollerServer
    _server.init(maxConn, epollTime);
}

void NetThread::addBindAdapter(BindAdapter* bind)
{  
    // 新建端口对象序号
    int uid = _seq.add(1);

    _listeners[uid] = bind;

}

void NetThread::initialize()
{
    // 设置回调函数
    _server.setCloseCallback(std::bind(&NetThread::closeEvent, this, _1));
    _server.setListenCallback(std::bind(&NetThread::acceptEvent, this, _1, _2));
    _server.setRecvCallback(std::bind(&NetThread::readEvent, this, _1, _2));
    
    try
    {
        // 添加监听端口
        auto it = _listeners.begin();
        for (; it != _listeners.end(); it++)
        {
            int fd = it->second->getBindFd();
            uint64_t data = std::int64_t(EpollerServer::EPOLL_ET_LISTEN) << 32;
            data |= it->first;
            // _ep.add(fd, data, EPOLLIN);
            
            _server.addListen(fd, data);
        }
        
        // 添加定时任务
        // 心跳检测

        // TODO: 试验一下
        // _server.addTimerTaskRepeat(0, _heartTime, &NetThread::HeartCheckTask, this);


    }
    catch (const Hrpc_Exception& e)
    {
        std::cerr << "[NetThread::initialize]: " << e.what() << ", errCode = " << e.getErrCode() << std::endl;
        throw;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        throw;
    }
    catch (...)
    {
        std::cerr << "[NetThread::initialize]: catch unknown exception" << std::endl;
        throw;
    }
    
}


void NetThread::run()
{
    try
    {
        //开启循环
        while (!_terminate)
        {
            // 运行epollSever核心循环
            _server.run();
    
        }
    }
    catch (const Hrpc_Exception& e)
    {
        std::cerr << "[NetThread::run]: catch exception--- " << e.what() << ", errno = " << e.getErrCode() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[NetThread::run]: catch exception--- " << e.what() << std::endl;
    }
    std::cout << "[NetThread::run]: netthread is end" << std::endl;
}

void NetThread::insertResponseQueue(ResponsePtr&& resp)
{
    _server.insertResponseQueue(std::move(resp));
}

void NetThread::notify()
{
    _server.notify();
}

void NetThread::terminate()
{
    _terminate = true;

    // 唤醒网络线程
    notify();
}


void NetThread::addConnection(const ConnectionPtr& ptr)
{
    // 这里将ptr添加到网络线程的监听列表中， 并且此操作在当前网络线程执行
    auto task = [ptr, this](){
        _server.addConnection(ptr);
    };
    
    runTaskBySelf(std::move(task));
    
    // 唤醒网络线程
    notify();
}

void NetThread::closeConnection(int uid)
{
    _server.closeConnection(uid);
    
}

void NetThread::HeartCheckTask()
{
    // 遍历一遍所有的connection， 寻找需要进行心跳检测的链接
    auto& _connections = _server.getConnections();
    for (auto x : _connections)
    {
        auto nowTime = Hrpc_Time::getNowTimeMs();
        
        auto diffTime = nowTime - x.second->getLastActivityTime();
        if (diffTime > _heartTime)
        {
            if (diffTime > _heartTime * 3)
            {
                std::cout << "[NetThread::closeConnection]: NetThread[" << std::this_thread::get_id()
                        << "], conneciton-id:" << x.second->getUid() << "  heartChecking timeout" << std::endl;

                closeConnection(x.second->getUid());
                continue;
            }

            // 进行心跳检测

            // 强制转化为TcpConnections
            auto tcpConn = std::dynamic_pointer_cast<TcpConnection>(x.second);
            tcpConn->sendHeartCheck();
        }
    }
}


void NetThread::acceptEvent(EpollerServer* server, const epoll_event& ev)
{
    int uid = ev.data.u32;

    // 新的链接到来
    if (ev.events & EPOLLIN)
    {
        auto itr = _listeners.find(uid);
        if (itr != _listeners.end())
        {
            // 通过BindAdapter接收新的链接
            while (itr->second->accept());
            // 判断链接已经被完全接受完成
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                std::cerr << "[NetThread::acceptEvent]: accept error, errno != EAGAIN or EWOULDBLOCK" << std::endl;
            }
        }
        else
        {
            std::cerr << "[NetThread::acceptEvent]: not found BindAdapter" << std::endl;
        }
    }
}   


void NetThread::readEvent(EpollerServer* server, const ConnectionPtr& conn)
{
    // 有新数据到来
    auto res = conn->recvData();
    bool isClose = false;
    if (res == -1)
    {
        // 对端关闭
        //做一个 关闭connection的标识， 这里不能直接关闭， 否则无法后面无法读取已经获得的数据
        isClose = true;
        
    }
    else if (res == -2)
    {
        isClose = true;
    }

    // 检测收到包的 完整性
    bool checkComplete = false;
    do 
    {
        Hrpc_Buffer tmp;
        checkComplete = conn->extractPackage(tmp);
        if (tmp.size() != 0)
        {
            // 有完整包，发往业务线程
            // conn->sendRequest(std::move(tmp));
            auto tcpConn = std::dynamic_pointer_cast<TcpConnection>(conn);
            tcpConn->sendRequest(std::move(tmp));
        }

    } while(checkComplete);

    // 判断是否需要关闭链接
    if (isClose)
    {
        closeConnection(conn->getUid());
        std::cout << "[NetThread::readEvent]: NetThread[" << std::this_thread::get_id()
                            << "], conneciton-id:" << conn->getUid() << " recv 0 bytes data, close connection" << std::endl;
    }
}

NetThread::~NetThread()
{
    terminate();
}
}