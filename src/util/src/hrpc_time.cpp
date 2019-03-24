#include <hrpc_time.h>

namespace Hrpc
{

size_t Hrpc_Time::getNowTime()
{
    return getNowTime() / 1000;
}

size_t Hrpc_Time::getNowTimeMs()
{
    timeval t;
    ::gettimeofday(&t, 0); 
    return t.tv_sec * 1000 + t.tv_usec / 1000;
}
}