/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @Date: 2019-03-21 16:06:58
 * @LastEditTime: 2019-03-24 14:47:56
 */
#include <cassert>
#include <hrpc_epoller.h>
namespace Hrpc
{
Hrpc_Epoller::Hrpc_Epoller(bool et) : _bEt(et),  _epoll_fd(-1), _pEvents(nullptr)
{    
}

void Hrpc_Epoller::createEpoll(size_t conn_num)
{
    if (_epoll_fd < 0)
    {
        _epoll_fd = epoll_create(conn_num);
        if (_epoll_fd < 0)
        {
            throw Hrpc_EpollerException("epoll_create error, errno saved", errno);
        }

        // 创建事件通知数组
        _max_connection = conn_num;
        if (_pEvents)
        {
            delete [] _pEvents;
        }
        try
        {
            _pEvents = new epoll_event[_max_connection];
        }
        catch (std::exception& e)
        {
            _pEvents = nullptr;
            throw Hrpc_EpollerException("epoll_create error, max-connction is too large");   
        }
        
    }
    else
    {
        throw  Hrpc_EpollerException("epoll_create error, epoll_fd  exist");
    }
}

void Hrpc_Epoller::add(int fd, uint64_t data, uint32_t event)
{
    assert(fd > 0);

    epoll_event ev;
    ev.data.u64 = data;
    ev.events = event;

    if(_bEt)
        ev.events |= EPOLLET;    
    
    ctrl(EPOLL_CTL_ADD, fd, &ev);
}


void Hrpc_Epoller::mod(int fd, uint64_t data, uint32_t event)
{
    assert(fd > 0);

    epoll_event ev;
    ev.data.u64 = data;
    ev.events = event;

    if(_bEt)
        ev.events |= EPOLLET;    
    
    ctrl(EPOLL_CTL_MOD, fd, &ev);
}


void Hrpc_Epoller::del(int fd, uint64_t data, uint32_t event)
{
    assert(fd > 0);

    // linux 2.6.8 以后event参数可以为NULL， 之前的必须为非空
    epoll_event ev;
    
    ctrl(EPOLL_CTL_DEL, fd, &ev);
}

void Hrpc_Epoller::ctrl(int op, int fd, epoll_event* event)
{
    int ret = epoll_ctl(_epoll_fd, op, fd, event);
    if (ret < 0)
        throw Hrpc_EpollerException("epoll_ctl error, errCode saved", errno);
}

int Hrpc_Epoller::wait(uint32_t millseconds)
{
    int ret = -1;
    // 若被信号中断， 则进行重试
    while ((ret = epoll_wait(_epoll_fd, _pEvents, _max_connection, millseconds)) < 0 && errno == EINTR);
    if (ret < 0)
        throw Hrpc_EpollerException("epoll_wait error, errCode saved", errno);
    return ret;
}

Hrpc_Epoller::~Hrpc_Epoller()
{
    // 释放fd以及数组
    if (_pEvents)
        delete[] _pEvents;
   
    if (_epoll_fd >= 0)
        ::close(_epoll_fd);
}
}