#ifndef HANDLEBASE_H_
#define HANDLEBASE_H_

#include <hrpc_buffer.h>

namespace Hrpc
{

/**
 *  Hrpc协议下面函数处理的基类
 *
 */
class HandleBase
{
public:
    /**
     * @description: 对于函数进行分发 
     * @param: funcName   函数名称
     * @param: msg        具体的消息包
     * @return: 
     */
    virtual Hrpc_Buffer distribute(const std::string& funcName, Hrpc_Buffer&& msg) = 0; 

    /**
     * @description: 获取处理hrpc协议的对象名称
     * @param {type} 
     * @return: 
     */
    virtual std::string getObjectName() const = 0;

    virtual ~HandleBase() = 0;
};
}
#endif