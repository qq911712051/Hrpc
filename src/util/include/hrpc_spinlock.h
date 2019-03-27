/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 自旋锁的简单实现
 * @Date: 2019-03-25 00:32:39
 */
#ifndef HRPC_SPINLOCK_H_
#define HRPC_SPINLOCK_H_

#include <hrpc_exception.h>
#include <pthread.h>

namespace Hrpc
{


class Hrpc_SpinlockException : public Hrpc_Exception
{
public:
    Hrpc_SpinlockException(const std::string& str) : Hrpc_Exception(str) {}
    Hrpc_SpinlockException(const std::string& str, int err) : Hrpc_Exception(str, err) {}
    ~Hrpc_SpinlockException() {}
};

/**
 * @description: 自旋锁 
 */
class Hrpc_Spinlock
{
public:
    /**
     * @description: 构造函数, 初始化保护变量 
     * @param {type} 
     * @return: 
     */
    Hrpc_Spinlock() : _protect(0), _tid(0) {}

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
     * @description: 尝试加锁，若不成功 
     * @param {type} 
     * @return: 
     */
    bool tryLock() const;

private:
    
    // 禁止赋值以及拷贝
    Hrpc_Spinlock(const Hrpc_Spinlock&);
    Hrpc_Spinlock& operator=(const Hrpc_Spinlock&);
private:
    mutable volatile int _protect; // 保护变量
    mutable volatile pthread_t _tid;         // 防止死锁
};

}

#endif
