/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description:  单例类
 * @Date: 2019-03-27 15:30:31
 */
#ifndef HRPC_SINGLETON_H_
#define HRPC_SINGLETON_H_

#include <hrpc_lock.h>

#define complier_barrier() __volatile__ __asm__("":::"memory")

namespace Hrpc
{
template <typename Object>
class Hrpc_Singleton
{
public:
    Hrpc_Singleton() : _pEle(0), _initialized(false) {}

    static Object* getInstance()
    {

        // 安全的双重检查锁
        if (!_initialized)
        {
            Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
            if (!_pEle)
            {
                _pEle = new Object();
                complier_barrier();
                _initialized = true;
            }
        }

        return _pEle;
    }
private:
    static Object* _pEle;
    bool _initialized;
    Hrpc_ThreadLock _lock;
};

}
#endif