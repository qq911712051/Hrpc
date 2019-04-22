#include <chrono>

#include <hrpc_time.h>

using namespace std::chrono;
namespace Hrpc
{

size_t Hrpc_Time::getNowTime()
{
    return getNowTimeMs() / 1000;
}

size_t Hrpc_Time::getNowTimeMs()
{
    // timeval t;
    // ::gettimeofday(&t, 0); 
    // return t.tv_sec * 1000 + t.tv_usec / 1000;

    auto dura = system_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(dura).count();
    
}


std::string Hrpc_Time::getTimeStamp()
{
    return tm2str(getNowTime());
}

std::string Hrpc_Time::tm2str(size_t seconds)
{

    
    char buffer[100] = {0};
    time_t tmp = seconds;
    
    // TODO: 这里需要优化， 存在锁竞争
    tm tmRes;
    ::localtime_r(&tmp, &tmRes);

    int res = ::strftime(buffer, 100, "%Y-%m-%d %H:%M:%S", &tmRes);
    return std::string(buffer);
}

}