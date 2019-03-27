#include <iostream>
#include <hrpc_atomic.h>
#include <hrpc_lock.h>
#include <hrpc_time.h>
#include <hrpc_thread.h>
#include <pthread.h>
#include <ctime>
// 测试
#include <mutex>
#include <thread>
using namespace Hrpc;

// Hrpc_Atomic g_atomic;
int g_atomic = 0;
std::mutex g_mutex;
Hrpc_MutexLock h_mutex;
Hrpc_RecMutexLock h_recMutex;
void func(size_t num)
{
    for (size_t i = 0; i < num; i++)
    {
        // std::lock_guard<std::mutex> l(g_mutex);
        // std::lock_guard<Hrpc_MutexLock> l(h_mutex);
        // std::lock_guard<Hrpc_RecMutexLock> l(h_recMutex);
        // Hrpc_LockGuard<std::mutex> l(g_mutex);
        // Hrpc_LockGuard<Hrpc_RecMutexLock> l(h_recMutex);
        Hrpc_LockGuard<Hrpc_MutexLock> l(h_mutex);
        g_atomic++;
    }
}

Hrpc_Atomic sources;
Hrpc_ThreadLock lock;


void func2(int i)
{
    while (true)
    {
        Hrpc_LockGuard<Hrpc_ThreadLock> sync(lock);
        if (sources.get() > 0)
        {
            sources--;
            std::cout << "thread " << i << " get sources" << std::endl;
            continue;
        }
        else
        {
            while (sources.get() <= 0)
            {
                lock.wait();
            }
        }
    }
}

class MyThread : public Hrpc_Thread
{
    int i;
public:
    void init(int index)
    {
        i = index;
    }
    void run()
    {
        while (true)
        {
            Hrpc_LockGuard<Hrpc_ThreadLock> sync(lock);
            if (sources.get() > 0)
            {
                sources--;
                std::cout << "thread " << i << " get sources" << std::endl;
                continue;
            }
            else
            {
                while (sources.get() <= 0)
                {
                    lock.wait();
                }
            }
        }
    }
};
int main()
{
/**
 *  测试Hrpc_ThreadLock 
 */    
    // std::thread threads[5];
    // for (size_t i =0; i < 5; i++)
    // {
    //     threads[i] = std::thread(func2, i);
    // }

    MyThread threads[5];
    for (size_t i =0; i < 5; i++)
    {
        threads[i].init(i);
        threads[i].start();
    }

    size_t count = 0;
    
    while (1)
    {
        {
            Hrpc_LockGuard<Hrpc_ThreadLock> sync(lock);
            sources++;
            lock.notifyAll();
        }
        ::usleep(1000);
    }



    // for (auto& t: threads)
    //     t.join();
    // std::cout << g_atomic << std::endl;    

    // pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    // pthread_mutex_t mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

    // pthread_mutex_lock(&mutex);
    // timespec tv;
    // size_t iSeconds = Hrpc_Time::getNowTime() + 1000;
    // tv.tv_sec = iSeconds;
    // tv.tv_nsec = 0;
    // int res = pthread_cond_timedwait(&cond, &mutex, &tv);
    // std::cout << "res = " << res << std::endl;
    
    // int lockRes = pthread_mutex_lock(&mutex);
    // std::cout << lockRes << std::endl;

    // std::cout << "now[" << Hrpc_Time::getTimeStamp() << "]" << std::endl;


    return 0;
}