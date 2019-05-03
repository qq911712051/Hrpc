/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 这里是HRPC解析协议文件 
 * @Date: 2019-04-27 16:36:11
 */
#ifndef HRPCPROTOCOL_H_
#define HRPCPROTOCOL_H_

#include <memory>

#include <hrpc_baseProtocol.h>

#include <HandleBase.h>
namespace Hrpc
{

/**
 * HRPC通信协议的具体实现， 数据的封装格式如下
 *  1bytes  协议名称长度N
 *  N bytes 协议名称
 *  4bytes  协议包序列号
 *  1bytes  协议状态( 请求状态， 响应状态， 单次调用等等)
 *  body   协议包体
 *  
 *      Hrpc协议包体的具体格式： 
 *  对象名称 + 调用函数 + 序列化的参数
 *   对象名称+调用函数通过  Object.FuncName  组合， 前面2字节存储这个字符串长度
 *  
 */
class HrpcProtocol : public Hrpc_BaseProtocol
{
public:
    enum
    {
        HRPC_REQUEST = 1, // 请求包
        HRPC_RESPONSE,    // 响应包
        HRPC_ONEWAY       // 单向请求包(不需要响应)
    };
public:
    HrpcProtocol() = default;
    /**
     * @description: 对于Hrpc协议进行解析, 服务端进行解析
     * @param:
     * @return: 
     */
    Hrpc_Buffer parse(Hrpc_Buffer&& buf) override;
    
    /**
     * @description: 设置 处理对象, 具体处理具体的业务 
     * @param: handle 处理对象
     * @return: 
     */
    void setHandleObject(std::unique_ptr<HandleBase>&& handle);

    /**
     * @description: 包装成一个Hrpc请求
     * @param: body Hrpc协议体 
     * @param: seq  hrpc请求包序号 
     * @param: type hrpc请求的类型 
     * @return: 
     */
    static Hrpc_Buffer makeRequest(Hrpc_Buffer&& body, int seq, std::int8_t type);

    /**
     * @description: 获取协议名称
     * @param {type} 
     * @return: 
     */
    static std::string getName() {return _name;};
    /**
     * @description: 析构函数
     * @param {type} 
     * @return: 
     */
    ~HrpcProtocol() override {}

private:
    
    /**
     * @description: 从buffer中提取出请求的对象名称， 和函数名称
     * @param: objectName 对象名称 
     * @param: objectName 函数名称 
     * @return: 返回是否提取成功
     */
    bool extractFuncName(Hrpc_Buffer&& msg, std::string& objectName, std::string& funcName);

    /**
     * @description: 添加协议头部信息
     *              包括1字节的协议长度， 然后协议名称， 再加上协议包编号
     * @param {type} 
     * @return: 
     */
    Hrpc_Buffer addProtocolHeadMsg(Hrpc_Buffer&& buf, std::int32_t seq);
private:
    std::unique_ptr<HandleBase> _handle;

    static std::string _name;   // 协议名称
};
}
#endif