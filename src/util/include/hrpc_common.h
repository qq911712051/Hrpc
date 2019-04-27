#ifndef HRPC_COMMON_H_
#define HRPC_COMMON_H_

#include <sstream>
#include <string>

#include <arpa/inet.h>
namespace Hrpc
{

class Hrpc_Common
{
public:
    /**
     * @description: 将字符串转化为其他数值型格式
     * @param {type} 
     * @return: 
     */
    template<typename Obj>
    static Obj strto(const std::string& str);

    /**
     * @description: 将数值类型转换为string类型 
     * @param {type} 
     * @return: 
     */
    template<typename Obj>
    static std::string tostr(Obj o);

    /**
     * @description: 编译器屏障
     * @param {type} 
     * @return: 
     */
    static void compiler_barrier()
    {
        asm volatile ("":::"memory");
    }

    /**
     * @description: 本地字节序转化为网络序   32位整数
     * @param {type} 
     * @return: 
     */
    static std::int32_t htonInt32(std::int32_t data)
    {
        return ::htonl(data);
    }

    /**
     * @description: 本地字节序转化为网络字节序  16位整数
     * @param {type} 
     * @return: 
     */
    static std::int16_t htonInt16(std::int16_t data)
    {
        return ::htons(data);
    }

    /**
     * @description: 网络字节序转化为本地字节序   32位整数
     * @param {type} 
     * @return: 
     */
    static std::int32_t ntohInt32(std::int32_t data)
    {
        return ::ntohl(data);
    }

    /**
     * @description: 网络字节序转化为本地字节序  16位整数
     * @param {type} 
     * @return: 
     */
    static std::int16_t ntohInt16(std::int16_t data)
    {
        return ::ntohs(data);
    }
};

template<typename Obj>
Obj Hrpc_Common::strto(const std::string& str)
{
    // 使用字符串流进行格式转换
    std::stringstream ss;
    ss << str;
    Obj tmp;
    ss >> tmp;
    return tmp;
}

template<typename Obj>
std::string Hrpc_Common::tostr(Obj o)
{
    std::stringstream ss;
    ss << o;
    std::string res;
    ss >> res;
    return res;
}
}
#endif