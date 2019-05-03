#include <ObjectProxy.h>
#include <ClientNetThreadGroup.h>

namespace Hrpc
{

ObjectProxy::ObjectProxy(ClientNetThreadGroup* group, const std::string& objName, const std::string& ip, short port)
{
    _net = group;
    _object = objName;
    _destIp = ip;
    _destPort = port;
}

Hrpc_Buffer ObjectProxy::involve(int type, const std::string& funcName, Hrpc_Buffer&& para)
{
    
}

void ObjectProxy::setWaitTime(size_t wait)
{
    if (wait > 0)
    {
        _waitTime = wait;
    }
}

}