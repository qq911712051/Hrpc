#include <iostream>

#include <sys/epoll.h>

#include <NetThread.h>
namespace Hrpc
{
void NetThread::UidGenarator::init(int maxConn)
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    for (size_t index = 1; index <= maxConn; index++)
    {
        _list.push_back(index);
    }
}
int NetThread::UidGenarator::popUid()
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    if (!_list.empty())
    {
        int tmp = _list.front();
        _list.pop_front();
        return tmp;
    }
    else
    {
        return -1;
    }
    
}
void NetThread::UidGenarator::pushUid(int uid)
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    _list.push_back(uid);
}
NetThread::NetThread(NetThreadGroup* ptr, int maxConn, int wait) : _threadGroup(ptr), _terminate(false), _Max_connections(maxConn), _waitTime(wait)
{
    _uidGen.init(maxConn);
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

    // 初始化bind端口
    bind->initialize();

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
        _ep.add(_notify.getFd(), EPOLL_ET_NOTIFY << 32, EPOLLIN);
        _ep.add(_shutdown.getFd(), EPOLL_ET_CLOSE << 32, EPOLLIN);

        // 添加监听端口
        typedef std::map<int, BindAdapter*>::iterator bind_iterator;
        bind_iterator it = _listeners.begin();
        for (; it != _listeners.end(); it++)
        {
            int fd = it->second->getBindFd();
            uint64_t data = EPOLL_ET_LISTEN << 32;
            data |= it->first;
            _ep.add(fd, data, EPOLLIN);
        }
        
        
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
    //开启循环

    while (!terminate)
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
                    acceptConnection(ev.data.u32);
                    break;
                }
                case EPOLL_ET_NET:
                {
                    processConnection(ev.data.u32);
                    break;
                }
                case EPOLL_ET_NOTIFY:
                {
                    break;
                }
                default:
                    std::cerr << "[NetThread::run]: switch type is unknown" << std::endl;
                    assert(false);
            };
        }        

        // 处理response队列
        processResponse();

        // do other thing
        // 定时器等操作
        _timer.process();
        
    }
    std::cout << "a NetThread end" << std::endl;
}


}