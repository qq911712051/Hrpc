#include <chrono>

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
    // 封装参数
    Hrpc_Buffer msg = makeHrpcBody(funcName, std::move(para));
    
    // 选择一个网络线程
    auto netPtr = _net->getNetThreadByRound();
    // 获取已经建立的链接，如果没有建立，则建立
    auto conn = netPtr->connect(_object, _destIp, _destPort);

    if (!conn)
    {
        throw Hrpc_Exception("[ObjectProxy::involve]: get invalid connection");
    }
    // 根据调用的类型
    if (type == HRPC_ONEWAY)
    {
        // 单向调用
        conn->call_func(std::move(msg));
    }
    else if (type == HRPC_FUNC)
    {
        // 普通的调用

        // 获取对应的fut
        auto fut = conn->call_func_with_future(std::move(msg));
        return waitResult(std::move(fut));
    }
    return Hrpc_Buffer();
}

void ObjectProxy::setWaitTime(size_t wait)
{
    if (wait > 0)
    {
        _waitTime = wait;
    }
}

Hrpc_Buffer ObjectProxy::makeHrpcBody(const std::string& funcName, Hrpc_Buffer&& buf)
{
    std::string dest = _object + "." + funcName;
    short length = dest.size();

    if (buf.beforeSize() >= length + 2)
    {
        buf.appendFront(dest);
        buf.appendFrontInt16(length);
        
        return std::move(buf);
    }

    Hrpc_Buffer res;
    res.appendInt16(length);
    res.write(dest);
    res.pushData(std::move(buf));

    // 返回结果
    return std::move(res);
}

Hrpc_Buffer ObjectProxy::waitResult(WaitFuture&& fut)
{   
    try
    {
        // 等待结果返回
        auto status = fut.wait_for(std::chrono::milliseconds(_waitTime));
        
        // 判断返回的状态
        if (status == std::future_status::timeout)
        {
            // 超时了
            throw Hrpc_Exception("hrpc involve timeout");
        }
        else if (status == std::future_status::ready)
        {
            // 数据准备好了
            return std::move(*fut.get());
        }
        else
        {
            throw Hrpc_Exception("future wait_for return_status is error");
        }
    }
    catch(const std::exception& e)
    {
        // 直接抛出去
        throw;
    }
}

}