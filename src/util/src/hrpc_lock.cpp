#include <hrpc_lock.h>

namespace Hrpc
{

Hrpc_MutexLock::Hrpc_MutexLock()
{
    // 初始化为error check类型的锁
    _mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;
    _count = 0;
}

Hrpc_RecMutexLock::Hrpc_RecMutexLock()
{
    // 初始化为error check类型的锁
    _mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
    _count = 0;
}

Hrpc_MutexLock::~Hrpc_MutexLock()
{
    // 释放mutex资源
    int res = pthread_mutex_destroy(&_mutex);
    if (res > 0)
    {
        // mutex还没有解锁释放  解锁
        pthread_mutex_unlock(&_mutex);
        
        // 释放
        pthread_mutex_destroy(&_mutex);
    }
    
}

Hrpc_RecMutexLock::~Hrpc_RecMutexLock()
{
    while (_count > 0 && _count--)
    {
        pthread_mutex_unlock(&_mutex);
    }
    // 释放mutex资源
    int res = pthread_mutex_destroy(&_mutex);
}


void Hrpc_MutexLock::lock() const
{
    int res = pthread_mutex_lock(&_mutex);
    if (res > 0)
    {
        if (res == EINVAL)
        {
            throw Hrpc_LockException("[Hrpc_MutexLock::lock]: error-check mutex uninitialied");
        }
        else
        {
            throw Hrpc_LockException("[Hrpc_MutexLock::lock]:error-check mutex  dead lock");
        }
    }
}

void Hrpc_RecMutexLock::lock() const
{
    int res = pthread_mutex_lock(&_mutex);
    if (res > 0)
    {
       throw Hrpc_LockException("[Hrpc_RecMutexLock::lock]: recurisive mutex uninitialied");
    }
    
    // 加锁数量增加
    _count++;
}

void Hrpc_MutexLock::unlock() const
{
    int res = pthread_mutex_unlock(&_mutex);
    if (res > 0)
    {
        if (res == EINVAL)
        {
            throw Hrpc_LockException("[Hrpc_MutexLock::unlock]: error-check mutex uninitialied");
        }
        else
        {
            throw Hrpc_LockException("[Hrpc_MutexLock::unlock]: error-check mutex not owned the lock");
        }
    }
}

void Hrpc_RecMutexLock::unlock() const
{
    int res = pthread_mutex_unlock(&_mutex);
    if (res > 0)
    {
        if (res == EINVAL)
        {
            throw Hrpc_LockException("[Hrpc_RecMutexLock::unlock]: recurisive mutex uninitialied");
        }
        else
        {
            throw Hrpc_LockException("[Hrpc_RecMutexLock::unlock]: recurisive mutex not owned the lock");
        }
    }

    // 加锁次数-1
    _count--;
}



bool Hrpc_MutexLock::tryLock() const
{
    int res = pthread_mutex_trylock(&_mutex);
    if (res > 0)
    {
        if (res == EINVAL)
        {
            throw Hrpc_LockException("[Hrpc_MutexLock::trylock]: error-check mutex uninitialied");
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool Hrpc_RecMutexLock::tryLock() const
{
    int res = pthread_mutex_trylock(&_mutex);
    if (res > 0)
    {
        if (res == EINVAL)
        {
            throw Hrpc_LockException("[Hrpc_RecMutexLock::trylock]: recurisive mutex uninitialied");
        }
        else
        {
            return false;
        }
    }

    // 加锁次数+1
    _count++;
    return true;
}


void Hrpc_ThreadLock::lock()
{
    _lock.lock();

    // 通知为0
    _count = 0;
}

void Hrpc_ThreadLock::unlock()
{
    // 发送通知
    emitAllNotify();

    _lock.unlock();
}

bool Hrpc_ThreadLock::tryLock()
{
    bool res = _lock.tryLock();
    if (res)
    {
        _count = 0;
    }
    return res;
}

void Hrpc_ThreadLock::wait()
{
    // wait调用后，锁会被释放，这里先提前发送所有通知信号
    emitAllNotify();
    _cond.wait(_lock);
}

bool Hrpc_ThreadLock::timeWait(int millseconds)
{
    emitAllNotify();
    return _cond.timeWait(_lock, millseconds);
}

void Hrpc_ThreadLock::notify()
{
    if (_count != -1)
        _count++;
}


void Hrpc_ThreadLock::notifyAll()
{
    _count = -1;
}

void Hrpc_ThreadLock::emitAllNotify()
{
    if (_count < 0)
    {
        // 广播信号
        _cond.broadcast();
        _count = 0;
    }
    else
    {
        while (_count > 0)
        {
            _cond.signal();
            _count--;
        }
    }
}

}