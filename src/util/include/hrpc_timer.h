/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 定时器相关 
 * @Date: 2019-04-01 00:27:55
 */

#ifndef HRPC_TIMER_H_
#define HRPC_TIMER_H_

#include <queue>
#include <functional>
#include <memory>
#include <set>
#include <map>

#include <hrpc_uid.h>
#include <hrpc_time.h>
namespace Hrpc
{

/**
 * 定时器类
 *  定时器精度为毫秒
 */
class Hrpc_Timer
{
public:
    using TimerId = int;    // 表示任务
private:
    enum
    {
        HRPC_ONCE = 1,      // 单次调用
        HRPC_REPEAT         // 循环调用
    };

    using Task = std::function<void()>;
    /**
     *  定时任务实体: 
     *      
     */
    struct TimerTask
    {
        size_t  __startTime     = {0}; // 任务执行的时间
        int     __type          = {0};     //  任务的类型
        int     __duration      = {0};// 若是循环任务， 循环周期
        Task    __task;                 //  任务对象
        bool    __terminate     = {false};    //  任务状态
        int     __id            = {0};// 当前uid
    };

    /**
     *  定时器任务优先级规则
     */
    struct TimerCmp
    {
        // 规则1: 过期时间短的优先
        // 规则2: 定时任务已经被终止的优先
        bool operator()(TimerTask* a, TimerTask* b)
        {
            return (a->__startTime < b->__startTime) || (a->__terminate && !b->__terminate);          
        }
    };
    using TimerTaskPtr = TimerTask*;
    using TimerQueue = std::multiset<TimerTaskPtr, TimerCmp>;
    using TimerMap = std::map<TimerId, TimerTaskPtr>;

public:

    Hrpc_Timer(int max = 1024)
    {
        _uid.init(max);
    }

    /**
     * @description: 禁止拷贝以及赋值
     * @param {type} 
     * @return: 
     */
    Hrpc_Timer(const Hrpc_Timer&) = delete;
    Hrpc_Timer& operator=(const Hrpc_Timer&) = delete;
    
    /**
     * @description: 执行定时器任务 
     * @param: base 时间基准点， 执行所有小于这个时间的任务
     *         若base为0， 表示以当前时间为基准点执行
     * @return: 
     */
    void process(size_t base = 0);

    /**
     * @description: 添加一次性任务
     * @param: after 多久之后执行  单位ms
     * @return: 
     */
    template <typename FuncObject>
    TimerId addTaskOnce(size_t after, FuncObject&& func);

    /**
     * @description: 添加重复任务
     * @param: after 多久之后执行  
     * @param: dura  重复周期
     * @return: 
     */
    template <typename FuncObject>
    TimerId addTaskRepeat(size_t after, size_t dura, FuncObject&& f);

    /**
     * @description: 停止任务执行
     * @param: id   定时任务的id
     * @return: 如果任务存在， 则返回true，
     *          否则返回false
     */
    bool stopTask(TimerId id);

private:

    /**
     * @description: 创建一个以base作为时间基准的任务对象， 用于比较
     * @param: base 时间基准
     * @return: 返回新建的任务对象 
     */
    std::shared_ptr<TimerTask> makeBaseTask(size_t base);

    /**
     * @description: 将任务从timerid的映射表中删除， 并delete整个任务对象
     * @param: task 具体的任务指针
     * @return: 
     */
    void deleteTask(TimerTaskPtr task);
private:
    TimerQueue      _queue;  // 定时器任务
    std::mutex      _lock;   // 保证任务队列线程安全

    TimerMap        _tasks;  // 存储timerid到具体TimerTask的映射
    std::mutex      _mapLock;   // 保证任务映射map线程安全

    UidGenarator    _uid;    // 生成TimerId   
};

template <typename FuncObject>
Hrpc_Timer::TimerId Hrpc_Timer::addTaskOnce(size_t after, FuncObject&& func)
{
    auto newTask = new TimerTask;
    newTask->__task = std::forward<FuncObject>(func);
    newTask->__startTime = Hrpc_Time::getNowTimeMs() + after;
    newTask->__type = HRPC_ONCE;
    
    // 获取新的任务id
    auto id = _uid.popUid();
    if (id < 0)
    {
        std::cerr << "[Hrpc_Timer::addTaskOnce]: timerId is not enougch" << std::endl;
        return -1;
    }
    newTask->__id = id;

    // 插入定时任务队列
    {
        std::lock_guard<std::mutex> sync(_lock);
        _queue.insert(newTask);
    }

    // 插入timerid映射表
    {
        std::lock_guard<std::mutex> sync(_mapLock);
        _tasks.emplace(id, newTask);
    }
    return id;
}

template <typename FuncObject>
Hrpc_Timer::TimerId Hrpc_Timer::addTaskRepeat(size_t after, size_t dura, FuncObject&& func)
{
    auto newTask = new TimerTask;
    newTask->__task = std::forward<FuncObject>(func);
    newTask->__startTime = Hrpc_Time::getNowTimeMs() + after;
    newTask->__type = HRPC_REPEAT;
    newTask->__duration = dura;
    // 获取新的任务id
    auto id = _uid.popUid();
    if (id < 0)
    {
        std::cerr << "[Hrpc_Timer::addTaskRepeat]: timerId is not enougch" << std::endl;
        return -1;
    }
    newTask->__id = id;

    // 插入定时任务队列
    {
        std::lock_guard<std::mutex> sync(_lock);
        _queue.insert(newTask);
    }

    // 插入timerid映射表
    {
        std::lock_guard<std::mutex> sync(_mapLock);
        _tasks.emplace(id, newTask);
    }
    return id;
}

}
#endif
