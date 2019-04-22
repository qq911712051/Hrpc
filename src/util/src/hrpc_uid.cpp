#include <cassert>

#include <hrpc_uid.h>

namespace Hrpc
{
void UidGenarator::init(int maxConn)
{
    if (maxConn < 0)
    {
        _MaxUid = 1024;
    }
    else 
    {
        _MaxUid = maxConn;
    }
    std::lock_guard<std::mutex> sync(_lock);
    for (size_t index = 1; index <= _MaxUid; index++)
    {
        _list.push_back(index);
    }
}
int UidGenarator::popUid()
{
    std::lock_guard<std::mutex> sync(_lock);
    if (!_list.empty())
    {
        int tmp = _list.front();
        _list.pop_front();
        return tmp;
    }
    else
    {
        return -1;
    }
    
}
void UidGenarator::pushUid(int uid)
{
    assert(uid > 0 && uid <= _MaxUid);
    std::lock_guard<std::mutex> sync(_lock);
    _list.push_back(uid);
}

}