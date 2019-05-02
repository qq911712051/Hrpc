#include <ObjectProxy.h>
#include <ClientNetThreadGroup.h>

namespace Hrpc
{

ObjectProxy::ObjectProxy(ClientNetThreadGroup* group, const std::string& ip, short port)
{
    _net = group;
    _destIp = ip;
    _destPort = port;
}

Hrpc_Buffer ObjectProxy::involve(int type, const std::string& funcName, Hrpc_Buffer&& para)
{
    
}

}