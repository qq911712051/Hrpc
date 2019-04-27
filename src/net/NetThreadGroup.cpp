#include <hrpc_common.h>

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

void NetThreadGroup::terminate()
{
    for (auto& t : _threads)
        t->terminate();
}
void NetThreadGroup::initialize(const Hrpc_Config& config)
{
    std::string data = config.getString("/hrpc/server/NetThread/ThreadNum");

    int res = Hrpc_Common::strto<int>(data);
    if (res <= 0)
    {
        res = 3;     // 若没有定义 
    }
    _thread_num = res;
    // 新建网络线程
    for (int i = 0; i < _thread_num; i++)
    {
        _threads.push_back(NetThreadPtr(new NetThread(this, config)));
    }
}

void NetThreadGroup::start()
{
    for (auto& x : _threads)
    {
        // 初始化
        x->initialize();
        // 启动
        x->start();
    }
}

NetThreadGroup::~NetThreadGroup()
{
    terminate();
    
    for (auto& t : _threads)
        t->join();
    
}

void NetThreadGroup::addBindAdapter(BindAdapter* bind)
{
    // 将bind添加到1号网络线程
    if (_threads.size() < 1)
    {
        throw Hrpc_Exception("[NetThreadGroup::addBindAdapter]: netThread num is zero");
    }
    // 添加监听端口
    _threads[0]->addBindAdapter(bind);
}
bool NetThreadGroup::isRunning()
{
    for (auto& t : _threads)
    {
        if (!t->isRunning())
            return false;
    }
    return true;
}
}