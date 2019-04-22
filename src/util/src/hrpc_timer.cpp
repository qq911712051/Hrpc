#include <iostream>
#include <mutex>
#include <vector>
#include <memory>
#include <cassert>

#include <hrpc_timer.h>
#include <hrpc_time.h>
namespace Hrpc
{


std::shared_ptr<Hrpc_Timer::TimerTask> Hrpc_Timer::makeBaseTask(size_t base)
{
    std::shared_ptr<TimerTask> ptr(new TimerTask);
    ptr->__startTime = base;
    return ptr;
}

void Hrpc_Timer::process(size_t base)
{
    if (base == 0)
    {
        base = Hrpc_Time::getNowTimeMs();
    }

    // 待处理的任务队列
    std::vector<TimerTaskPtr> eq; 

    {
        auto destTask = makeBaseTask(base);
        std::lock_guard<std::mutex> sync(_lock);
        auto upper_itr = _queue.lower_bound(destTask.get());

        if(upper_itr != _queue.begin())
        {
            // 取出所有的过期任务
            eq.insert(eq.end(), _queue.begin(), upper_itr);
            _queue.erase(_queue.begin(), upper_itr);
        }
        else 
        {
            return;
        }
    }

    std::vector<TimerTaskPtr> newTask;  // 新产生的任务队列
    // 执行相关任务
    for (auto& x : eq)
    {
        // 若 任务被取消
        if (x->__terminate)
        {
            deleteTask(x);
            continue;
        }
        // 处理定时任务
        switch (x->__type)
        {
            case HRPC_ONCE:
            {
                x->__task();
                
                // 消除任务
                deleteTask(x);
                break;
            }
            case HRPC_REPEAT:
            {
                x->__task();
                x->__startTime = Hrpc_Time::getNowTimeMs() + x->__duration;
                // 将新任务压入任务队列
                newTask.push_back(x);
                break;
            }
            default:
                std::cerr << "[Hrpc_Timer::process]: unknown task type" << std::endl;
        };  
        
    }
    // 将新任务插入定时任务序列
    if (!newTask.empty())
    {
        std::lock_guard<std::mutex> sync(_lock);
        _queue.insert(newTask.begin(), newTask.end());
    }
}


bool Hrpc_Timer::stopTask(TimerId id)
{
    TimerTaskPtr task = nullptr;
    {
        std::lock_guard<std::mutex> sync(_mapLock);
        auto itr = _tasks.find(id);
        if (itr != _tasks.end())
        {
            task = itr->second;
        }
        else 
            return false;
    }

    // 将任务从定时任务队列中取出,重新压入， 进行排序
    {
        std::lock_guard<std::mutex> sync(_lock);
        auto range = _queue.equal_range(task);
        
        // 找到相关timerTask。删除
        for (auto itr = range.first; itr != range.second; itr++)
        {
            if ((*itr)->__id == id)
            {
                // 删除目标任务
                _queue.erase(itr);
                break;
            }
            if (itr == range.second)
            {
                std::cerr << "[Hrpc_Timer::stopTask]: occur error, id invalid" << std::endl;
                return false;
            }
        }

        task->__terminate = true;

        // 重新插入任务队列
        _queue.insert(task);
    }
    return true;
}


void Hrpc_Timer::deleteTask(TimerTaskPtr task)
{
    assert(task != nullptr);

    auto id = task->__id;
    // 删除timerid映射
    {
        std::lock_guard<std::mutex> sync(_mapLock);
        _tasks.erase(id);
    }

    // 归还id
    _uid.pushUid(id);

    // 释放定时任务对象的内存
    delete task;
}

}