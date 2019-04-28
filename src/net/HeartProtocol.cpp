#include <iostream>

#include <hrpc_time.h>

#include <HeartProtocol.h>


namespace Hrpc
{
std::string HeartProtocol::_name = "HEART"; // 定义心跳协议的名称

Hrpc_Buffer HeartProtocol::parse(Hrpc_Buffer&& buf)
{
    std::int8_t status;
    Hrpc_Buffer body;
    if (extract(std::move(buf), status, body))   
    {

        switch (status)
        {
            case HEART_RESPONSE:
            {
                if (body.size() == 4)
                {
                    std::int32_t time = body.peekFrontInt32();
                    
                    std::cout << "[HeartProtocol::parse]: get Heart-Protocol response, sender time [" << Hrpc_Time::tm2str(time) << "]" << std::endl;
                }
                else
                {
                    std::cerr << "[HeartProtocol::parse]: occur error, heartProtocol body size != 4" << std::endl;
                }
                
                break;
            }
            default:
            {
                std::cerr << "[HeartProtocol::parse]: unknown status-type = " << status << std::endl;
            }
        };
    }
    else
    {
        std::cerr << "[HeartProtocol::parse]: request package error, [" << buf.toByteString() << "]" << std::endl;
    }
    return Hrpc_Buffer();
}

Hrpc_Buffer HeartProtocol::doRequest()
{
    Hrpc_Buffer res;
    std::int8_t status = HEART_REQUEST;
    std::int8_t length = _name.size();

    res.appendInt8(length); // 写入名称长度
    res.write(_name);   // 写入名称
    res.appendInt8(status); // 写入请求状态

    return res;
}


bool HeartProtocol::extract(Hrpc_Buffer&& msg, std::int8_t& status, Hrpc_Buffer& res)
{
    if (msg.size() != 0)
    {
        // 取得状态
        status = msg.peekFrontInt8();
        
        msg.skipBytes(1);

        res = std::move(msg);
    }
    else
    {
        return false;
    }
    return true;
}

}