/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 封装了pthread提供的条件变量 
 * @Date: 2019-03-25 23:05:20
 */

#ifndef HRPC_CONDITION_H_
#define HRPC_CONDITION_H_
#include <pthread.h>
#include <hrpc_exception.h>
namespace Hrpc
{

/**
 *  条件变量异常类 
 */
class Hrpc_ConditonException : public Hrpc_Exception
{
public:
    Hrpc_ConditonException(const std::string& str) : Hrpc_Exception(str) {}
    Hrpc_ConditonException(const std::string& str, int err) : Hrpc_Exception(str, err) {}
    ~Hrpc_ConditonException() {}
};


/**
 * 条件变量， 封装基本的操作
 */
class Hrpc_Condition
{
public:
    /**
     * @description: 初始化条件变量
     * @param {type} 
     * @return: 
     */
    Hrpc_Condition();

    /**
     * @description: 释放条件变量资源
     * @param {type} 
     * @return: 
     */
    ~Hrpc_Condition();

    /**
     * @description: 唤醒一个正在等待的线程
     * @param {type} 
     * @return: 
     */
    void signal();

    /**
     * @description: 唤醒全部等待的线程
     * @param {type} 
     * @return: 
     */
    void broadcast();
    
    /**
     * @description: 永久等待直到被唤醒
     * @param: lock 互斥锁
     * @return: 
     */
    template <typename MutexType> void wait(MutexType& lock);

    /**
     * @description: 等待millseconds秒
     * @param: millseconds, 等待时间， 单位毫秒
     * @return: 若返回真， 
     */
    template <typename MutexType> bool timeWait(MutexType& lock, int millseconds);

private:
    // 禁止拷贝以及赋值
    Hrpc_Condition(const Hrpc_Condition&);
    Hrpc_Condition& operator=(const Hrpc_Condition&);

    /**
     * @description: 获取在当前时间点经过millseconds之后的绝对时间
     * @param: millseconds 毫秒
     * @return: 获取目标时间点的绝对时间
     */
    timespec getAbsTime(int millseconds);
private:
    pthread_cond_t _cond;      // 条件变量
};

template <typename MutexType> 
void Hrpc_Condition::wait(MutexType& lock)
{
    int tmp = lock._count; // 对于可重入锁来说， 保存其 加锁次数

    pthread_cond_wait(&_cond, &lock._mutex);
    // 恢复加锁次数 , only for recurisive mutex
    lock._count = tmp;
}

template <typename MutexType> 
bool Hrpc_Condition::timeWait(MutexType& lock, int millseconds)
{
    if (millseconds < 0)
    {
        throw Hrpc_ConditonException("[Hrpc_Condition::timeWait]: millseconds must be greater than zero");
    }
    int tmp = lock._count;
    
    // 计算超时时间
    timespec ts = getAbsTime(millseconds);
    // 是否超时
    int res = pthread_cond_timedwait(&_cond, &lock._mutex, &ts);

    if (res != 0)
    {
        if (res == ETIMEDOUT)
        {
            lock._count = tmp;
            return false;
        }
        else
        {
            // 遇到EINTR错误,直接抛异常
            throw Hrpc_ConditonException("[Hrpc_Condition::timeWait]: pthread_cond_timedwait error, errno saved", res);
        }
    }
    lock._count = tmp;
    return true;
}

}

#endif
