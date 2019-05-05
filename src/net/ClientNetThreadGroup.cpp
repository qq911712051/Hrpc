#include <hrpc_common.h>

#include <ClientNetThreadGroup.h>

namespace Hrpc
{

void ClientNetThreadGroup::intialize(const Hrpc_Config& config)
{
    _hash = 0;

    // 从config读取参数
    int threadNum = Hrpc_Common::strto<int>(config.getString("/hrpc/client/NetThread/ThreadNum"));
    if (threadNum <= 0)
    {
        std::cerr << "[ClientNetThreadGroup::intialize]: not found ThreadNum config, set default = 1" << std::endl;
        threadNum = 1;
    }
    _num = threadNum;
    std::cout << "[ClientNetThreadGroup::intialize]: intialize the client netThread number is " << _num << std::endl;
    // 新建网络线程
    for (int i = 0; i < _num; i++)
    {
        _threads.push_back(ClientNetThreadPtr(new ClientNetThread(this)));
    }

    // 初始化
    for (auto& t : _threads)
    {
        t->intialize(config);
    }
}

void ClientNetThreadGroup::start()
{
    for (auto& t : _threads)
    {
        t->start();
    }
}

ClientNetThread* ClientNetThreadGroup::getNetThreadByRound()
{
    int num = _hash.add(1);
    if (num > 21000000)
        _hash = 0;
    
    return _threads[num % _num].get();
}

void ClientNetThreadGroup::terminate()
{
    for (auto& t : _threads)
    {
        t->terminate();
    }
}
ClientNetThreadGroup::~ClientNetThreadGroup()
{
    terminate();
    
    for (auto& t : _threads)
    {
        t->join();
    }
}
}