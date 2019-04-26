/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 队列实现类， 包括线程安全队列， 无锁队列等等
 * @Date: 2019-03-30 16:26:38
 */
#ifndef HRPC_QUEUE_H_
#define HRPC_QUEUE_H_
#include <queue>
#include <vector>
#include <deque>

#include <hrpc_lock.h>
namespace Hrpc
{

/**
 * 线程安全的并发队列
 */
template <typename ObjectType, template <class, class> class BaseContainer = std::queue>
class Hrpc_Queue
{
public:
    typedef size_t size_type;
    typedef ObjectType element_type;
    typedef BaseContainer<element_type, std::deque<element_type>> queue_type;
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
     * @description: 优化插入, 可以接受右值
     * @param {type} 
     * @return: 
     */
    void push(ObjectType&& ele);
    
    /**
     * @description: 从队列中取出一个对象
     * @param: ele 目标对象
     * @param: timeout 超时时间, 单位ms
     *          若小于0， 表示阻塞等待，直到取出对象
     *          若等于0， 表示若队列中不为空， 则取出对象，没有直接返回
     *          若大于0， 表示等待特定的时间， 超时失败
     * @return: 
     *
     */
    bool pop(ObjectType& ele, int timeout = -1);

    /**
     * @description: 阻塞,直到有元素可以取出 
     *             可以用作移动的对象, 比如std::unique_ptr
     * @param {type} 
     * @return: 
     */
    ObjectType pop(int timeout= -1);

    
    
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
     * @return: 
     */
    void swap(const Hrpc_Queue& q);

    /**
     * @description: 将当前队列中所有内容 快速交换 到tmp中
     * @param:  timeout 阻塞时间
     * @return: 
     */
    void swap(std::queue<ObjectType>& tmp, int timeout = -1);

    /**
     * @description: 获取队列中元素的个数
     * @param {type} 
     * @return: 返回元素数量
     */
    size_type size();

private:

private:
    mutable Hrpc_ThreadLock _lock;  // 保证线程安全
    mutable queue_type _queue;  // 对象容器
};

template <typename ObjectType, template <class,class> class BaseContainer>
void Hrpc_Queue<ObjectType, BaseContainer>::push(const ObjectType& ele)
{
    {
        Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
        _queue.push(ele);
        _lock.notify();     // 通知
    }
}

template <typename ObjectType, template <class,class> class BaseContainer>
void Hrpc_Queue<ObjectType, BaseContainer>::push(ObjectType&& ele)
{
    {
        Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
        _queue.push(std::forward<ObjectType>(ele));
        _lock.notify();     // 通知
    }
}

template <typename ObjectType, template <class,class> class BaseContainer>
bool Hrpc_Queue<ObjectType, BaseContainer>::pop(ObjectType& ele, int timeout)
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    bool flag = false;  // 表示是否已经等待一次
    while (_queue.empty())
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

template <typename ObjectType, template <class,class> class BaseContainer>
ObjectType Hrpc_Queue<ObjectType, BaseContainer>::pop(int timeout)
{
    ObjectType tmp;
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    bool flag = false;  // 表示是否已经等待一次
    while (_queue.empty())
    {
        if (!flag)
            flag = true;
        else
            return tmp;
        bool res = _lock.timeWait(timeout);
        if (!res)
        {
            return tmp;
        }
        
    }
    // 取到新元素
    tmp = std::move(_queue.front());
    _queue.pop();
    return tmp;
}
    
template <typename ObjectType, template <class,class> class BaseContainer>
void Hrpc_Queue<ObjectType, BaseContainer>::clear()
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    _queue.clear();
}
    

template <typename ObjectType, template <class,class> class BaseContainer>
bool Hrpc_Queue<ObjectType, BaseContainer>::empty()
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    return _queue.empty();
}
    
template <typename ObjectType, template <class,class> class BaseContainer>
void Hrpc_Queue<ObjectType, BaseContainer>::swap(const Hrpc_Queue& q)
{
    std::lock<Hrpc_ThreadLock, Hrpc_ThreadLock>(_lock, q._lock);
    
    std::lock_guard<Hrpc_ThreadLock> lock_1(_lock, std::adopt_lock);
    std::lock_guard<Hrpc_ThreadLock> lock_2(q._lock, std::adopt_lock);

    _queue.swap(q._queue);

}

template <typename ObjectType, template <class,class> class BaseContainer>
void Hrpc_Queue<ObjectType, BaseContainer>::swap(std::queue<ObjectType>& q, int timeout)
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    while (_queue.empty())
    {
        if (timeout < 0)
            _lock.wait();
        else
        {
            bool res = _lock.timeWait(timeout);
            if (!res)
            {
                return;
            }
        }   
    }
    _queue.swap(q);
}

template <typename ObjectType, template <class,class> class BaseContainer>
typename Hrpc_Queue<ObjectType, BaseContainer>::size_type Hrpc_Queue<ObjectType, BaseContainer>::size()
{
    Hrpc_LockGuard<Hrpc_ThreadLock> sync(_lock);
    return _queue.size();
}


}
#endif