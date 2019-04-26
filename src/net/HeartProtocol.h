/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 心跳协议的相关操作
 * @Date: 2019-04-26 14:50:27
 */
#ifndef HEARTPROTOCOL_H_
#define HEARTPROTOCOL_H_

#include <hrpc_baseProtocol.h>

namespace Hrpc
{

/**
 * 心跳协议　：　具体格式为
 *      // 1byte协议名称长度 + 协议名称 +　1bytes协议状态　＋　协议体
 */
class HeartProtocol : public Hrpc_BaseProtocol
{
    enum 
    {
        HEART_REQUEST = 1,  // 协议包 请求状态
        HEART_RESPONSE      // 协议包 响应状态
    };
public:
    HeartProtocol() = default;
    /**
     * @description: 对于Hrpc协议进行解析
     * @param:
     * @return: 
     */
    Hrpc_Buffer parse(Hrpc_Buffer&& buf) override;
    
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
    ~HeartProtocol() override {};

    /**
     * @description: 进行一次心跳请求
     * @param {type} 
     * @return: 
     */
    static Hrpc_Buffer doRequest();
private:
    /**
     * @description: 响应心跳协议
     * @param {type} 
     * @return: 
     */
    static Hrpc_Buffer doResponse();

    /**
     * @description: 提取msg消息包中一些信息
     * @param: msg 原始消息包
     * @param: status 消息包的状态
     * @param: res 消息体
     * @return: 
     */
    static bool extract(Hrpc_Buffer&& msg, std::int8_t& status, Hrpc_Buffer& res);

private:
    static std::string _name;
};
std::string HeartProtocol::_name = "HEART"; // 定义心跳协议的名称
}
#endif