#include <iostream>
#include <cstring>
#include <fstream>

#include <parser.h>
using namespace Hrpc;
int main(int argc, char* argv[])
{
    // 开始解析文件
    std::fstream fin(argv[1]);


    if (!fin.is_open())
    {
        return 0;
    }

    // buffer
    char buffer[1024] = {0};
    std::string data;
    while (fin.getline(buffer, 1024))
    {
        std::size_t length = ::strlen(buffer);
        // 是否有注释符号#
        for (int i = 0; i < length; i++)
        {
            if (buffer[i] == '#')
            {
                buffer[i] = '\0';  // 忽略后面数据
                break;
            }
        }
        // 压入到有效数据中
        data.append(buffer);
    }
    
    auto vec = Scanner::scan(data);
    std::cout << "total size = " << vec.size() << std::endl;
    for (auto& x : vec)
    {
        Scanner::toString(x);
    }
    auto res = Parser::parse(vec);
    std::cout << "=============================================" << std::endl;
    std::cout << res << std::endl;
    return 0;
}
