#include <iostream>
#include <cstring>
#include <fstream>

#include <parser.h>
using namespace Hrpc;

bool check(const std::string& fileName, std::string& object)
{
    if (fileName.size() >= 6)
    {
        int pos = fileName.size() - 5;
        if (fileName[pos] == '.' && fileName[pos+1] == 'h' && fileName[pos+2] == 'r' && fileName[pos+3] == 'p' && fileName[pos+4] == 'c')
        {
            object = std::string(fileName.begin(), fileName.begin() + pos);
            return true;
        }
    }
    return false;
}

int main(int argc, char* argv[])
{
    std::string object;
    if (!check(argv[1], object))
    {
        std::cerr << "filename is error" << std::endl;
        std::cerr << "usage: program filename" << std::endl;
        return 0;
    }
    // 开始解析文件
    std::fstream fin(argv[1]);


    if (!fin.is_open())
    {
        std::cerr << "open file error" << std::endl;
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

    auto res = Parser::parse(vec);
    
    // 新建文件
    std::string newFile = object + ".h";
    std::fstream fout(newFile, std::ios_base::in | std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
    if (!fout.is_open())
    {
        std::cerr << "make file " << newFile << "  error" << std::endl;
        return 0;
    }
    fout.write(res.data(), res.size());
    fout.close();
    fin.close();
    return 0;
}
