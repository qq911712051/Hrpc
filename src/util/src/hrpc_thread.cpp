#include <hrpc_thread.h>
#include <iostream>

namespace Hrpc
{

void Hrpc_Thread::start()
{
    if (_running)
        return;
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    int res = pthread_create(&_tid, 0, enterFunc, this);
    if (res != 0)
    {
        // 恢复tid
        _tid = 0;
        throw Hrpc_ThreadException("[Hrpc_Thread::start]: pthread_create error, errno saved", res);
    }

    // 等待新线程正式开始运行
    _lock.wait();

}

void Hrpc_Thread::join()
{
    int res = pthread_join(_tid, 0);
    if (res != 0)
        throw Hrpc_ThreadException("[Hrpc_Thread::join]:pthread_join error, errno saved", res);
}

void Hrpc_Thread::detach()
{
    int res = pthread_detach(_tid);
    if (res != 0)
        throw Hrpc_ThreadException("[Hrpc_Thread::detach]:pthread_detach error, errno saved", res);
}


void* Hrpc_Thread::enterFunc(void* ptr)
{
    Hrpc_Thread* self = static_cast<Hrpc_Thread*>(ptr);
    try
    {
        // 通知调用线程，新线程已经拉起
        {
            Hrpc_LockGuard<Hrpc_ThreadLock> sync(self->_lock);
            self->_running = true;
            self->_lock.notify();
        }
        // 运行派生类实现函数
        self->run();
    }
    catch (const Hrpc_Exception& e)
    {
        std::cerr << e.what() << ", errCode = " << e.getErrCode() << std::endl;
        throw;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[Hrpc_Thread::enterFunc]: " << e.what() << std::endl;
        throw;
    }
    catch (...)
    {
        std::cerr << "[Hrpc_Thread::enterFunc]: unknown exception " << std::endl;
        throw;
    }
    

    // 线程已经停止运行
    self->_running = false;
    return 0;
}

Hrpc_Thread::~Hrpc_Thread()
{

}

}