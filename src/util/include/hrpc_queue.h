/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 队列实现类， 包括线程安全队列， 无锁队列等等
 * @Date: 2019-03-30 16:26:38
 */
#ifndef HRPC_QUEUE_H_
#define HRPC_QUEUE_H_
#include <hrpc_lock.h>
#include <queue>
namespace Hrpc
{

/**
 * 线程安全的并发队列
 */
template <typename ObjectType, template <class> class BaseContainer = std::queue>
class Hrpc_Queue
{
public:
    typedef size_t size_type;
    typedef ObjectType element_type;
    typedef BaseContainer<elementType> queue_type;
public:
    Hrpc_Queue() {}
    ~Hrpc_Queue() {}
    /**
     * @description: 向队列中插入对象
     * @param: ele 目标对象
     * @return: 
     */
    void push(const ObjectType& ele);
    
    /**
     * @description: 从队列中取出一个对象
     * @param: ele 目标对象
     * @param: timeout 超时时间, 单位ms
     *          若小于0， 表示阻塞等待，直到取出对象
     *          若等于0， 表示若队列中不为空， 则取出对象，没有直接返回
     *          若大于0， 表示等待特定的时间， 超时失败
     * @return: 
     *          成功取到对象返回true， 否则超时返回fasle
     */
    bool pop(ObjectType& ele, int timeout = -1);

    
    /**
     * @description: 清空队列
     * @param {type} 
     * @return: 
     */
    void clear();
    
    /**
     * @description: 是否为空
     * @param {type} 
     * @return: 返回是否队列为空
     */
    bool empty();

    /**
     * @description: 快速交换
     * @param {type} 
     * @return: 返回是否交换成功， 要同时锁住2个锁， 如果发生死锁，则返回false
     */
    bool swap(const Hrpc_Queue& q);

    /**
     * @description: 获取队列中元素的个数
     * @param {type} 
     * @return: 返回元素数量
     */
    size_type size();

private:

private:
    Hrpc_ThreadLock _lock;  // 保证线程安全
    mutable queue_type _queue;  // 对象容器
};

template <typename ObjectType, template <class> class BaseContainer>
void Hrpc_Queue<ObjectType, BaseContainer>::push(const ObjectType& ele)
{
    {
        Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
        _queue.push(ele);
        _lock.notify();     // 通知
    }
}

template <typename ObjectType, template <class> class BaseContainer>
bool Hrpc_Queue<ObjectType, BaseContainer>::pop(ObjectType& ele, int timeout)
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    bool flag = false;  // 表示是否已经等待一次
    while (!_queue.empty())
    {
        if (!flag)
            flag = true;
        else
            return false;
        bool res = _lock.timeWait(timeout);
        if (!res)
        {
            return false;
        }
        
    }
    // 取到新元素
    ele = _queue.front();
    _queue.pop();
    return true;
}
    
template <typename ObjectType, template <class> class BaseContainer>
void Hrpc_Queue<ObjectType, BaseContainer>::clear()
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    _queue.clear();
}
    

template <typename ObjectType, template <class> class BaseContainer>
bool Hrpc_Queue<ObjectType, BaseContainer>::empty()
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    return _queue.empty();
}
    
template <typename ObjectType, template <class> class BaseContainer>
bool Hrpc_Queue<ObjectType, BaseContainer>::swap(const Hrpc_Queue& q)
{
    // 防止死锁
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    bool res = q._lock.tryLock();
    if (!res)
        return false;

    _queue.swap(q._queue);

    // 这里进行解锁
    q._lock.unlock();
    return true;
}

template <typename ObjectType, template <class> class BaseContainer>
Hrpc_Queue<ObjectType, BaseContainer>::size_type Hrpc_Queue<ObjectType, BaseContainer>::size()
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    return _queue.size();
}


}
#endif