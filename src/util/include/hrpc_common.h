#ifndef HRPC_COMMON_H_
#define HRPC_COMMON_H_

#include <sstream>
#include <string>
namespace Hrpc
{

class Hrpc_Common
{
public:
    template<typename Obj>
    static Obj strto(const std::string& str);
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
}
#endif