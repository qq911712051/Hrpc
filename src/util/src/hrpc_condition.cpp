/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description:  条件变量的实现
 * @Date: 2019-03-25 23:05:30
 */

#include <hrpc_condition.h>
#include <iostream>

#include <hrpc_time.h>
namespace Hrpc
{
Hrpc_Condition::Hrpc_Condition()
{
    _cond = PTHREAD_COND_INITIALIZER;
}


Hrpc_Condition::~Hrpc_Condition()
{
    int res = pthread_cond_destroy(&_cond);
    if (res)
    {
        // 出错
        std::cerr << "[Hrpc_Condition::~Hrpc_Condition]: pthread_cond_destroy error" << std::endl;
    }
}

void Hrpc_Condition::signal()
{
    pthread_cond_signal(&_cond);
}

void Hrpc_Condition::broadcast()
{
    pthread_cond_broadcast(&_cond);
}
timespec Hrpc_Condition::getAbsTime(int millseconds)
{
    size_t now = Hrpc_Time::getNowTimeMs();
    now += millseconds;
    timespec ts;
    ts.tv_sec = now / 1000;
    ts.tv_nsec = (now % 1000) * 1000 * 1000;
    return ts;
}
}