#include <memory>
#include <thread>

#include <hrpc_exception.h>

#include <EpollerServer.h>

namespace Hrpc
{

void EpollerServer::init(size_t maxConn, int waitTime)
{
    if (maxConn <= 0)
    {
        _Max_connections = 1024;
    }
    else
    {
        _Max_connections = maxConn;
    }

    if (waitTime < 0)
    {
        _waitTime = 10;
    }
    else
    {
        _waitTime = waitTime;
    }

    // 初始化uid生成
    _uidGen.init(_Max_connections);

    _notify.CreateSocket(SOCK_STREAM, AF_INET);
    _shutdown.CreateSocket(SOCK_STREAM, AF_INET);

    // 初始化epoll
    _ep.createEpoll(_Max_connections);

    // 添加唤醒socket以及关闭socket
    _ep.add(_notify.getFd(), std::int64_t(EPOLL_ET_NOTIFY) << 32, EPOLLIN);
    _ep.add(_shutdown.getFd(), std::int64_t(EPOLL_ET_CLOSE) << 32, EPOLLIN);
    
}

void EpollerServer::run()
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
                if (_closeCallback)
                {
                    _closeCallback(this);
                }
                else
                {
                    throw Hrpc_Exception("[EpollerServer::run]: closeEventCallback is null");
                }
                
                break;
            }
            case EPOLL_ET_LISTEN:
            {
                // acceptConnection(ev);
                if (_listenCallback)
                {
                    _listenCallback(this, ev);
                }
                else
                {
                    throw Hrpc_Exception("[EpollerServer::run]: listenEventCallback is null");
                }
                break;
            }
            case EPOLL_ET_NET:
            {
                // 处理数据的收发
                processConnection(ev);
                break;
            }
            case EPOLL_ET_NOTIFY:
            {
                // 什么也不做，仅仅将网络线程从阻塞状态中唤醒
                break;
            }
            default:
                std::cerr << "[EpollerServer::run]: unknown MESSAGE-type [" << (ev.data.u64 >> 32) << "]" << std::endl;
        };
    }        

    // 处理response队列
    processResponse();

    // 处理定时任务 以及 一些 网路线程任务
    _timer.process();
            
}

void EpollerServer::processResponse()
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
        
        ConnectionPtr conn;
        auto itr = _connections.find(task->_uid);
        if (itr == _connections.end() && task->_type != ResponseMessage::HRPC_RESPONSE_TASK)
        {
            std::cerr << "[EpollerServer::processResponse]: TcpConnection weak_ptr is invalid" << std::endl;
            continue;
        }
        else
        {
            conn = itr->second;
        }
        

        switch (task->_type)
        {
            case ResponseMessage::HRPC_RESPONSE_CLOSE:
            {
                closeConnection(conn->getUid());
                std::cout << "[EpollerServer::processResponse]: server close the connection!" << std::endl;
                break;
            }
            case ResponseMessage::HRPC_RESPONSE_NET:
            {
                // 处理当前链接需要发送的数据
                auto buf = std::move(task->_buffer);

                // 在这里发送数据
                // 如果connection待发送缓冲区中还留存数据 则将 此数据压入connection的待发送缓冲区， 并监听对应socket的可写状态
                // 如果connection待发送缓冲区中没有数据  则将此数据直接通过socket发送
                if (conn->sendBufferSize() == 0)
                {
                    // bool res =conn->sendData(std::move(*buf));
                    conn->pushData(std::move(*buf));
                    int res = conn->sendData();
                    if (res == -1)
                    {
                        // 对端关闭链接
                        closeConnection(conn->getUid());
                        std::cout << "[EpollerServer::processResponse]: NetThread[" << std::this_thread::get_id()
                                    << "], conneciton-id:" << conn->getUid() << " send 0 bytes data, close connection" << std::endl;
                    }
                    else if (res == -3)
                    {
                        // 异常情况
                        std::cerr << "[EpollerServer::processResponse]: occur error, send return -1, and errno = " << errno << std::endl;
                        closeConnection(conn->getUid());
                    }
                    else if (res == -2) // 数据还未发送完成
                    {
                        // 监听链接是否可写
                        _ep.mod(conn->getFd(), (std::int64_t(EPOLL_ET_NET) << 32) | conn->getUid(), EPOLLIN | EPOLLOUT);    
                    }
                }
                else
                {
                    // 新数据压到旧数据末尾
                    conn->pushData(std::move(*buf));
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
                    std::cerr << "[EpollerServer::processResponse]: when execute a task, the task-pointer is null" << std::endl;
                }
                break;
            }
            default:
            {
                std::cerr << "[EpollerServer::processResponse]: unkown response type" << std::endl;
            }
        };
    }
}

void EpollerServer::setListenCallback(ListenEventCallback&& callback)
{
    _listenCallback = std::move(callback);
}
void EpollerServer::setCloseCallback(CloseEventCallback&& callback)
{
    _closeCallback = std::move(callback);
}
void EpollerServer::setRecvCallback(RecvDataCallback&& callback)
{
    _recvCallback = std::move(callback);
}

void EpollerServer::notify()
{
    // 修改notify事件
    std::int64_t data = std::int64_t(EPOLL_ET_NOTIFY) << 32;
    _ep.mod(_notify.getFd(), data, EPOLLOUT);
}

void EpollerServer::insertResponseQueue(ResponsePtr&& resp)
{
    // 插入到待处理队列
    _response_queue.push(std::move(resp));

    // 唤醒等待的网络线程
    notify();
}

void EpollerServer::closeConnection(int uid)
{
    auto itr = _connections.find(uid);
    if (itr != _connections.end())
    {
        auto conn = itr->second;
        std::cout << "[EpollerServer::closeConnection]: NetThread[" << std::this_thread::get_id()
                        << "], conneciton-id:" << conn->getUid() << "  closed" << std::endl;

        // 设置此链接无效
        conn->setClosed();
        // 从epoll中删除对于此链接的监听
        _ep.del(conn->getFd());
        // 从connections链表中删除此链接
        _connections.erase(conn->getUid());

        // 归还uid
        _uidGen.pushUid(uid);
    }
    else
    {
        std::cerr << "[EpollerServer::closeConnection]: invalid uid " << std::endl;
    }
}

void EpollerServer::addConnection(const ConnectionPtr& conn)
{
    // 获取uid
    auto id = _uidGen.popUid();
    if (id < 0)
    {
        std::cerr << "[EpollerServer::addConnection]: uid is not enough" << std::endl;
    }

    // 设置uid
    conn->setUid(id);
    // 添加到connection表
    _connections[id] = conn;

    // 将当前链接添加到ep监听中
    _ep.add(conn->getFd(), (std::int64_t(EPOLL_ET_NET)<<32) | id, EPOLLIN);
}

void EpollerServer::addListen(int fd, size_t data)
{
    _ep.add(fd, data, EPOLLIN);
}

void EpollerServer::processConnection(epoll_event ev)
{
    int uid = ev.data.u32;

    auto conn = _connections.find(uid);
    if (conn != _connections.end())
    {
        // 发生异常, 对端异常关闭
        if ((ev.events & EPOLLERR) || (ev.events & EPOLLHUP))
        {
            std::cerr << "[EpollerServer::processConnection]: event is EPOLLERR or EPOLLHUP, client occur error" << std::endl;
            closeConnection(uid);
            return ;
        }

        // 对端正常关闭链接或者是 shutdown关闭写
        if (ev.events & EPOLLRDHUP)
        {
            std::cout << "[EpollerServer::processConnection]: peer closed" << std::endl;
            closeConnection(uid);
            return;
        }

        // 进行数据的收发工作
        if (ev.events & EPOLLIN)
        {
            if (_recvCallback)
            {
                _recvCallback(this, conn->second);
            }
            else
            {
                throw Hrpc_Exception("[EpollerServer::processConnection]: recv callback is null");
            }
            
        }
        if (ev.events & EPOLLOUT)
        {
            // 发送数据
            int res = conn->second->sendData();
            if (res == -1)
            {
                // 对端关闭链接
                closeConnection(conn->second->getUid());
                std::cout << "[EpollerServer::processConnection]: NetThread[" << std::this_thread::get_id()
                            << "], conneciton-id:" << conn->second->getUid() << " send 0 bytes data, close connection" << std::endl;
            }
            else if (res == -3)
            {
                // 异常情况
                std::cerr << "[EpollerServer::processConnection]: occur error, send return -1, and errno = " << errno << std::endl;
                closeConnection(conn->second->getUid());
            }
            else if (res == 0)
            {
                // 删除对于可写事件的监听
                _ep.mod(conn->second->getFd(), (std::int64_t(EPOLL_ET_NET) << 32) | conn->second->getUid(), EPOLLIN);
            }
        }
    }
    else
    {
        std::cerr << "[EpollerServer::processConnection]: invalid uid " << std::endl;
    }
    
}


}