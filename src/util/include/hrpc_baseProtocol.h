#ifndef HRPC_BASE_PROTOCOL_H_
#define HRPC_BASE_PROTOCOL_H_

#include <hrpc_exception.h>
#include <hrpc_buffer.h>

namespace Hrpc
{


/**
 * 协议出错时 抛出错误
 */
class Hrpc_ProtocolException : public Hrpc_Exception
{
public:
    Hrpc_ProtocolException(const std::string& msg) : Hrpc_Exception(msg) {}
    Hrpc_ProtocolException(const std::string& msg, int err) : Hrpc_Exception(msg, err) {}
    ~Hrpc_ProtocolException() {}
    
};

class Hrpc_BaseProtocol
{
public:
    /**
     * @description: 协议的解析函数
     * @param: 待解析的包体
     * @return: 返回一个需要返回的信息
     */
    virtual Hrpc_Buffer parse(Hrpc_Buffer&& buf);

    /**
     * @description: 获取协议的名称
     * @param {type} 
     * @return: 
     */
    virtual std::string getName() = 0;

    /**
     * @description: 虚析构函数
     * @param {type} 
     * @return: 
     */
    virtual ~Hrpc_BaseProtocol() {}
};

}
#endif 