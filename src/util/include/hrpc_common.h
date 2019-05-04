#ifndef HRPC_COMMON_H_
#define HRPC_COMMON_H_

#include <sstream>
#include <string>
#include <cstring>

#include <arpa/inet.h>
namespace Hrpc
{

class Hrpc_Common
{
    union Check_Helper
    {
        int a;
        char b;
    };
public:
    /**
     * @description: 检测是不是小端
     * @param {type} 
     * @return: 
     */
    static bool isLittleEndian()
    {
        Check_Helper t;
        t.a = 1;
        if (t.b == 1)
        {
            return true;
        }
        return false;
    }

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
     * @description: 本地字节序转化为网络序   64/位整数
     * @param {type} 
     * @return: 
     */
    static std::int64_t htonInt64(std::int64_t data)
    {
        if (!isLittleEndian())
        {
            return data;
        }
        else
        {
            // 小端转化为大端
            std::int32_t l = htonInt32(*((std::int32_t*)&data));
            std::int32_t r = htonInt32(*((std::int32_t*)&data + 1));
            
            // 构造结果
            std::int64_t res = 0;
            ::memcpy((void*)&res, (void*)&r, sizeof(std::int32_t));
            ::memcpy((void*)((std::int32_t*)&res + 1), (void*)&l, sizeof(std::int32_t));
            return res;
        }
    }

    /**
     * @description: 网络字节序转化为本地   64/位整数
     * @param {type} 
     * @return: 
     */
    static std::int64_t ntohInt64(std::int64_t data)
    {
        if (!isLittleEndian())
        {
            return data;
        }
        else
        {
            // 小端转化为大端
            std::int32_t l = ntohInt32(*((std::int32_t*)&data));
            std::int32_t r = ntohInt32(*((std::int32_t*)&data + 1));
            
            // 构造结果
            std::int64_t res = 0;
            ::memcpy((void*)&res, (void*)&r, sizeof(std::int32_t));
            ::memcpy((void*)((std::int32_t*)&res + 1), (void*)&l, sizeof(std::int32_t));
            return res;
        }
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