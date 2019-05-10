#include <hrpc_common.h>

namespace Hrpc
{
std::string Hrpc_Common::toUpper(const std::string& data)
{   
    std::string res;
    for (auto& x : data)
    {
        if (x >= 'a' && x <= 'z')
        {
            res += (x - ('a' - 'A'));
        }
        else
        {
            res += x;
        }
        
    }
    return res;
}

}