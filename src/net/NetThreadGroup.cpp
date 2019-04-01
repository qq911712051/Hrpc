#include <NetThreadGroup.h>

namespace Hrpc
{

NetThread* NetThreadGroup::getNetThreadByRound()
{
    int hash = _hash.add(1);

    // 避免无限制自增以后导致的溢出情况
    if (hash > 42000)
        _hash = 0;
    
    size_t threadIndex = hash % _thread_num;
    return _threads[threadIndex].get();
}
}