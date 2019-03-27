/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 和锁相关的类定义， 包括互斥锁，重入锁，条件变量等等 
 * @Date: 2019-03-25 12:45:19
 */


#ifndef HRPC_LOCK_H_
#define HRPC_LOCK_H_

#include <hrpc_exception.h>
#include <hrpc_condition.h>

namespace Hrpc
{

/**
 * @description: 锁相关异常类
 * @param {type} 
 * @return: 
 */
class Hrpc_LockException : public Hrpc_Exception
{
public:
    Hrpc_LockException(const std::string& str) : Hrpc_Exception(str) {}
    Hrpc_LockException(const std::string& str,int err) : Hrpc_Exception(str, err) {}
    ~Hrpc_LockException() {}
};


/**
 * @description: 使用RAII手法封装, 监管锁
 * @param {type} 
 * @return: 
 */
template <typename LockType>
class Hrpc_LockGuard
{
public:
    /**
     * @description: 加锁 
     * @param {type} 
     * @return: 
     */
    Hrpc_LockGuard(LockType& lock);
    ~Hrpc_LockGuard();

    /**
     * @description:  加锁
     * @param {type} 
     * @return: 
     */
    void lock();

    /**
     * @description: 解锁
     * @param {type} 
     * @return: 
     */
    void unlock();

    /**
     * @description: 尝试加锁
     * @param {type} 
     * @return: 
     */
    bool tryLock();    

private:    
    LockType& _lock; // 锁类型对象
    bool _locked;       // 是否上锁
};


/**
 * @description: 互斥锁(不可重入), 只允许同一个线程进行加解锁(error-check mutex)
 *      基于pthread线程库中通过的API
 */
class Hrpc_MutexLock
{
    friend class Hrpc_Condition;
public:
    /**
     * @description:  构造函数， 初始化mutex变量
     * @param {type} 
     * @return: 
     */
    Hrpc_MutexLock();

    /**
     * @description: 释放mutex资源
     * @param {type} 
     * @return: 
     */
    ~Hrpc_MutexLock();

    /**
     * @description: 加锁 
     * @param {type} 
     * @return: 
     */
    void lock() const;

    /**
     * @description:  解锁
     * @param {type} 
     * @return: 
     */
    void unlock() const;

    /**
     * @description: 尝试解锁 
     * @param {type} 
     * @return: 解锁成功返回true，失败返回false
     */
    bool tryLock() const;
private:

    // 禁止赋值以及拷贝
    Hrpc_MutexLock(const Hrpc_MutexLock&);
    Hrpc_MutexLock& operator=(const Hrpc_MutexLock&);
private:
    mutable pthread_mutex_t _mutex; // 锁变量
    int _count; // 为了和Hrpc_Condition进行兼容, 没有任何卵用
};



/**
 * @description: 可重入互斥锁, mutex类型为 RECURISIVE
 *      基于pthread线程库中通过的API
 */
class Hrpc_RecMutexLock
{
    friend class Hrpc_Condition;
public:
    /**
     * @description:  构造函数， 初始化mutex变量
     * @param {type} 
     * @return: 
     */
    Hrpc_RecMutexLock();

    /**
     * @description: 释放mutex资源
     * @param {type} 
     * @return: 
     */
    ~Hrpc_RecMutexLock();

    /**
     * @description: 加锁 
     * @param {type} 
     * @return: 
     */
    void lock() const;

    /**
     * @description:  解锁
     * @param {type} 
     * @return: 
     */
    void unlock() const;

    /**
     * @description: 尝试解锁 
     * @param {type} 
     * @return: 解锁成功返回true，失败返回false
     */
    bool tryLock() const;
private:

    // 禁止赋值以及拷贝
    Hrpc_RecMutexLock(const Hrpc_RecMutexLock&);
    Hrpc_RecMutexLock& operator=(const Hrpc_RecMutexLock&);
private:
    mutable pthread_mutex_t _mutex; // 锁变量
    mutable int _count;         // 同一线程加锁的次数
};

/**
 *  线程锁， 将mutex和condition进行封装 
 *      加锁后进行操作， 通知信号将在解锁的时候发出
 */
class Hrpc_ThreadLock
{
public:
    /**
     * @description: 初始化
     * @param {type} 
     * @return: 
     */
    Hrpc_ThreadLock() : _count(0) {}

    ~Hrpc_ThreadLock() {}
    
    /**
     * @description: 加锁 
     * @param {type} 
     * @return: 
     */
    void lock();

    /**
     * @description: 解锁 
     * @param {type} 
     * @return: 
     */
    void unlock();
    
    /**
     * @description: 尝试加锁 
     * @param {type} 
     * @return: 
     */
    bool tryLock();

    /**
     * @description: 阻塞等待，直到通知到来
     * @param {type} 
     * @return: 
     */
    void wait();

    /**
     * @description: 等待有限时间， 没有时间返回 
     * @param： millseconds 等待时间
     * @return: 若为true， 表示通知唤醒
     *          若为false， 表示超时
     */
    bool timeWait(int millseconds);
    
    /**
     * @description: 唤醒一个等待线程 
     * @param {type} 
     * @return: 
     */
    void notify();

    /**
     * @description: 唤醒所有等待线程
     * @param {type} 
     * @return: 
     */
    void notifyAll();

private:

    /**
     * @description: 解锁时， 发送所有通知 
     * @param {type} 
     * @return: 
     */
    void emitAllNotify();

    // 禁止拷贝赋值
    Hrpc_ThreadLock(const Hrpc_ThreadLock&);
    Hrpc_ThreadLock& operator=(const Hrpc_ThreadLock&);

private:
    Hrpc_Condition _cond;       // 信号量
    Hrpc_RecMutexLock _lock;    // 可重入互斥锁

    int _count;     // 需要通知的次数
};




template <typename LockType> 
Hrpc_LockGuard<LockType>::Hrpc_LockGuard(LockType& lock) : _lock(lock)
{
    _lock.lock();
    
    _locked = true;
}


template <typename LockType> 
Hrpc_LockGuard<LockType>::~Hrpc_LockGuard()
{
    if (_locked)
        unlock();    
}


template <typename LockType> 
void Hrpc_LockGuard<LockType>::lock()
{
    if (_locked)
        return;
    try
    {
        _lock.lock();
        _locked = true;
    }
    catch (const std::exception& e)
    {
    }
}

template <typename LockType> 
void Hrpc_LockGuard<LockType>::unlock()
{
    try
    {
        _lock.unlock();
        _locked = false;
    }
    catch (const std::exception& e)
    {
    }
}



}
#endif
