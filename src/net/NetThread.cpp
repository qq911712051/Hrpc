#include <iostream>
#include <thread>

#include <sys/epoll.h>

#include <hrpc_common.h>

#include <NetThread.h>
namespace Hrpc
{

// NetThread::NetThread(NetThreadGroup* ptr, int maxConn, int wait, int heartTime) : _threadGroup(ptr), _terminate(false), _Max_connections(maxConn), _waitTime(wait), _heartTime(heartTime)
NetThread::NetThread(NetThreadGroup* ptr, const Hrpc_Config& config) : _threadGroup(ptr), _terminate(false)
{
    std::string data = config.getString("/hrpc/server/NetThread/MaxConnection");
    int res = Hrpc_Common::strto<int>(data);
    if (res <= 0)
    {
        res = 1024;     // 若没有定义， 则默认1024
    }
    _Max_connections = res;
    std::cout << "NetThread [" << std::this_thread::get_id() << "] set MaxConnection param [" << res << "]" << std::endl;

    _uidGen.init(res);

    // epoll时间
    data = config.getString("/hrpc/server/NetThread/EpollWaitTime");

    res = Hrpc_Common::strto<int>(data);
    if (res <= 0)
    {
        res = 10;
    }
    _waitTime = res;

    std::cout << "NetThread [" << std::this_thread::get_id() << "] set EpollWaitTime param [" << res << "]" << std::endl;
    // 心跳协议时间
    data = config.getString("/hrpc/server/NetThread/HeartTime");

    res = Hrpc_Common::strto<int>(data);
    if (res <= 1000)
    {
        res = 1000;
    }
    _heartTime = res;
    std::cout << "NetThread [" << std::this_thread::get_id() << "] set HeartTime param [" << res << "]" << std::endl;
}

void NetThread::addBindAdapter(BindAdapter* bind)
{  
    // 获取uid
    int uid = _uidGen.popUid();
    if (uid < 0)
    {
        throw Hrpc_NetThreadException("[NetThread::addBindAdapter]: uid is null, connection is too large");
    }
    _listeners[uid] = bind;

}

void NetThread::initialize()
{
    try
    {
        _notify.CreateSocket(SOCK_STREAM, AF_INET);
        _shutdown.CreateSocket(SOCK_STREAM, AF_INET);

        // 初始化epoll
        _ep.createEpoll(_Max_connections);

        // 添加唤醒socket以及关闭socket
        _ep.add(_notify.getFd(), std::int64_t(EPOLL_ET_NOTIFY) << 32, EPOLLIN);
        _ep.add(_shutdown.getFd(), std::int64_t(EPOLL_ET_CLOSE) << 32, EPOLLIN);

        // 添加监听端口
        typedef std::map<int, BindAdapter*>::iterator bind_iterator;
        bind_iterator it = _listeners.begin();
        for (; it != _listeners.end(); it++)
        {
            int fd = it->second->getBindFd();
            uint64_t data = std::int64_t(EPOLL_ET_LISTEN) << 32;
            data |= it->first;
            _ep.add(fd, data, EPOLLIN);
        }
        
        // 添加定时任务
        // 心跳检测
        auto heartCheck = std::bind(&NetThread::HeartCheckTask, this);
        _timer.addTaskRepeat(0, _heartTime, heartCheck);
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
            // epoll等待
            size_t nums = _ep.wait(_waitTime);
            
            for (size_t i = 0; i < nums; i++)
            {
                epoll_event ev = _ep.get(i);
                // 根据事件类型分类处理
                switch (ev.data.u64 >> 32)
                {
                    case EPOLL_ET_CLOSE:
                    {
                        closeEvent();
                        break;
                    }
                    case EPOLL_ET_LISTEN:
                    {
                        acceptConnection(ev);
                        break;
                    }
                    case EPOLL_ET_NET:
                    {
                        processConnection(ev);
                        break;
                    }
                    case EPOLL_ET_NOTIFY:
                    {
                        // 什么也不做，仅仅将网络线程从阻塞状态中唤醒
                        break;
                    }
                    default:
                        std::cerr << "[NetThread::run]: unknown MESSAGE-type [" << (ev.data.u64 >> 32) << "]" << std::endl;
                };
            }        

            // 处理response队列
            processResponse();

            // 处理定时任务 以及 一些 网路线程任务
            _timer.process();
            
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
    // 插入到待处理队列
    _response_queue.push(std::move(resp));

    // 唤醒等待的网络线程
    notify();
}

void NetThread::notify()
{
    // 修改notify事件
    std::int64_t data = std::int64_t(EPOLL_ET_NOTIFY) << 32;
    _ep.mod(_notify.getFd(), data, EPOLLOUT);
}

void NetThread::terminate()
{
    _terminate = true;

    // 唤醒网络线程
    notify();
}


void NetThread::acceptConnection(epoll_event ev)
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
                std::cerr << "[NetThread::acceptConnection]: accept error, errno != EAGAIN or EWOULDBLOCK" << std::endl;
            }
        }
        else
        {
            std::cerr << "[NetThread::acceptConnection]: not found BindAdapter" << std::endl;
        }
    }

}

void NetThread::addConnection(const TcpConnectionPtr& ptr)
{
    // 这里将ptr添加到网络线程的监听列表中， 并且此操作在当前网络线程执行
    auto task = [ptr, this](){
        // 获取uid
        auto id = this->_uidGen.popUid();
        if (id < 0)
        {
            std::cerr << "[NetThread::addConnectionBySelf]: uid is not enough" << std::endl;
        }

        // 设置uid
        ptr->setUid(id);
        // 添加到connection表
        this->_connections[id] = ptr;

        // 将当前链接添加到ep监听中
        this->_ep.add(ptr->getFd(), (std::int64_t(EPOLL_ET_NET)<<32) | id, EPOLLIN);
        
    };
    
    runTaskBySelf(std::move(task));
    
    // 唤醒网络线程
    notify();
}

void NetThread::processConnection(epoll_event ev)
{
    int uid = ev.data.u32;

    auto conn = _connections.find(uid);
    if (conn != _connections.end())
    {
        // 发生异常, 对端异常关闭
        if ((ev.events & EPOLLERR) || (ev.events & EPOLLHUP))
        {
            std::cerr << "[NetThread::processConnection]: event is EPOLLERR or EPOLLHUP, client occur error" << std::endl;
            closeConnection(uid);
            return ;
        }

        // 对端正常关闭链接或者是 shutdown关闭写
        if (ev.events & EPOLLRDHUP)
        {
            std::cout << "[NetThread::processConnection]: peer closed" << std::endl;
            closeConnection(uid);
            return;
        }

        // 进行数据的收发工作
        if (ev.events & EPOLLIN)
        {
            // 有新数据到来
            conn->second->recvData();
        }
        if (ev.events & EPOLLOUT)
        {
            // 发送数据
            int res = conn->second->sendData();
            if (res == 0)
            {
                // 删除对于可写事件的监听
                _ep.mod(conn->second->getFd(), (std::int64_t(EPOLL_ET_NET) << 32) | conn->second->getUid(), EPOLLIN);
            }
        }
    }
    else
    {
        std::cerr << "[NetThread::processConnection]: invalid uid " << std::endl;
    }
    
}


void NetThread::processResponse()
{
    // 快速获取所有的待处理任务
    std::queue<ResponsePtr> taskQueue;
    _response_queue.swap(taskQueue, 0);

    // 处理任务
    while (!taskQueue.empty())
    {
        // 取出元素
        auto task = std::move(taskQueue.front());
        taskQueue.pop();
        
        auto conn = task->_connection.lock();
        if (!conn && task->_type != ResponseMessage::HRPC_RESPONSE_TASK)
        {
            std::cerr << "[NetThread::processResponse]: TcpConnection weak_ptr is invalid" << std::endl;
            continue;
        }

        switch (task->_type)
        {
            case ResponseMessage::HRPC_RESPONSE_CLOSE:
            {
                closeConnection(conn->getUid());
                std::cout << "[NetThread::processResponse]: server close the connection!" << std::endl;
                break;
            }
            case ResponseMessage::HRPC_RESPONSE_NET:
            {
                // 处理当前链接需要发送的数据
                auto buf = std::move(task->_buffer);

                // 在这里发送数据
                // 如果connection待发送缓冲区中还留存数据 则将 此数据压入connection的待发送缓冲区， 并监听对应socket的可写状态
                // 如果connection待发送缓冲区中没有数据  则将此数据直接通过socket发送
                if (conn->isLastSendComplete())
                {
                    // bool res =conn->sendData(std::move(*buf));
                    conn->pushDataInSendBuffer(std::move(*buf));
                    int res = conn->sendData();
                    if (res == -2) // 数据还未发送完成
                    {
                        // 监听链接是否可写
                        _ep.mod(conn->getFd(), (std::int64_t(EPOLL_ET_NET) << 32) | conn->getUid(), EPOLLIN | EPOLLOUT);    
                    }
                }
                else
                {
                    // 新数据压到旧数据末尾
                    conn->pushDataInSendBuffer(std::move(*buf));
                }
                break;
            }
            case ResponseMessage::HRPC_RESPONSE_TASK:
            {
                // 执行任务
                if (task->_task)
                {
                    (*task->_task)();
                }
                else
                {
                    std::cerr << "[NetThread::processResponse]: when execute a task, the task-pointer is null" << std::endl;
                }
                break;
            }
            default:
            {
                std::cerr << "[NetThread::processResponse]: unkown response type" << std::endl;
            }
        };
    }
    
}

void NetThread::closeConnection(int uid)
{
    auto itr = _connections.find(uid);
    if (itr != _connections.end())
    {
        auto conn = itr->second;
        std::cout << "[NetThread::closeConnection]: NetThread[" << std::this_thread::get_id()
                        << "], conneciton-id:" << conn->getUid() << "  closed" << std::endl;
        // 从epoll中删除对于此链接的监听
        _ep.del(conn->getFd());
        // 从connections链表中删除此链接
        _connections.erase(conn->getUid());

        // 归还uid
        _uidGen.pushUid(uid);
    }
    else
    {
        std::cerr << "[NetThread::closeConnection]: invalid uid " << std::endl;
    }
    
}

void NetThread::HeartCheckTask()
{
    // 遍历一遍所有的connection， 寻找需要进行心跳检测的链接
    for (auto& x : _connections)
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
            x.second->sendHeartCheck();
        }
    }
}

NetThread::~NetThread()
{
    terminate();
}
}