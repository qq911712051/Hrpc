#include <iostream>

#include <HrpcProtocol.h>

namespace Hrpc
{
// 定义此协议的名称
std::string HrpcProtocol::_name = "HRPC";

Hrpc_Buffer HrpcProtocol::parse(Hrpc_Buffer&& buf)
{
    if (buf.size() < 6)
    {
        std::cerr << "[HrpcProtocol::parse]: hrpc package error, context = [" << buf.toByteString() << "]" << std::endl;
        return Hrpc_Buffer();
    }

    // 获取序列号
    auto seq = buf.peekFrontInt32();
    buf.skipBytes(4);

    // 获取协议状态
    auto status = buf.peekFrontInt8();
    buf.skipBytes(1);
    
    std::string objectName, funcName;
    if (extractFuncName(std::move(buf), objectName, funcName))
    {
        // 判断对象名称是否符合
        if (objectName != _handle->getObjectName())
        {
            std::cerr << "[HrpcProtocol::parse]: can't handle the Object [" << objectName << "], only deal with [" << _handle->getObjectName() << "]" << std::endl;
            return Hrpc_Buffer();
        }

        switch (status)
        {
            case HRPC_REQUEST:
            {
                // 对于普通请求
                auto retBuf = _handle->distribute(funcName, std::move(buf));
                // 对于返回包， 添加协议头
                return addProtocolHeadMsg(std::move(retBuf), seq);
            }
            case HRPC_ONEWAY:
            {
                // 对于单向请求
                _handle->distribute(funcName, std::move(buf));
                return Hrpc_Buffer();
            }
            default:
            {
                std::cerr << "[HrpcProtocol::parse]: hrpc package error, unknown package status = [" << status << "]" << std::endl;
            }
        };
    }
    else
    {
        std::cerr << "[HrpcProtocol::parse]: hrpc package error, can't load the funcName, context = [" << buf.toByteString() << "]" << std::endl;
    }
    return Hrpc_Buffer();
}


void HrpcProtocol::setHandleObject(std::unique_ptr<HandleBase>&& handle)
{
    _handle = std::move(handle);
}

bool HrpcProtocol::extractFuncName(Hrpc_Buffer&& msg, std::string& objectName, std::string& funcName)
{
    if (msg.size() < 1)
        return false;
    
    auto length = msg.peekFrontInt16();
    if (length <= 1)
        return false;
    
    std::string data = msg.get(2, length);

    // 将获得字符串拆成对象名称和函数名称
    auto pos = data.find(".");
    if (pos != std::string::npos)
    {
        objectName = std::string(data.begin(), data.begin() + pos);
        funcName = std::string(data.begin() + pos + 1, data.end());
    }
    else
    {
        return false;
    }

    // 读取前面字符
    msg.skipBytes(2 + length);
    return true;
}

Hrpc_Buffer HrpcProtocol::addProtocolHeadMsg(Hrpc_Buffer&& buf, std::int32_t seq)
{
    // 1bytes 协议名称长度 + 协议的名称
    if (buf.beforeSize() >= 5 + _name.size())
    {
        buf.appendFrontInt32(seq);
        buf.appendFront("HRPC");
        buf.appendFrontInt8(4);
        return std::move(buf);
    }
    else
    {
        Hrpc_Buffer res;
        res.appendInt8(4);
        res.write("HRPC");
        res.appendInt32(seq);
        res.pushData(std::move(buf));

        return std::move(res);
    }
    return Hrpc_Buffer();
}

}