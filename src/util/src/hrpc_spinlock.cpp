/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 自旋锁的实现
 * @Date: 2019-03-25 00:43:50
 */
#include <hrpc_spinlock.h>

#define complier_barrier()  __asm__ __volatile__("":::"memory")

namespace Hrpc
{

void Hrpc_Spinlock::lock() const
{
    pthread_t now = pthread_self();
    if (now != _tid)
    {
        bool res = false;
        do
        {
            if (_protect)
                continue;
            
            res = __sync_bool_compare_and_swap(&_protect, 0, 1);
        } while (!res);
        complier_barrier();
        _tid = now;
    }
    else
    {
        throw Hrpc_SpinlockException("[Hrpc_Spinlock::lock]: dead lock error");
    }
    
}

void Hrpc_Spinlock::unlock() const
{
    if (_protect == 1)
    {
        _tid = 0;
        
        complier_barrier();

        _protect = 0;
    }
    else
    {
        throw Hrpc_SpinlockException("[Hrpc_Spinlock::unlock]: unlock error");
    }
}

bool Hrpc_Spinlock::tryLock() const
{
    bool res =  __sync_bool_compare_and_swap(&_protect, 0, 1);
    if (res)
    {
        _tid = pthread_self();
        return true;
    }
    else
        return false;
}
}