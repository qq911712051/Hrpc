#include <iostream>
#include "hrpc_epoller.h"
#include <hrpc_exception.h>
using namespace Hrpc;
int main(int argc, char* argv[])
{
    Hrpc::Hrpc_Epoller ep;
    ep.createEpoll(1024);
    try
    {
        throw Hrpc::Hrpc_Exception("测试错误", -2);
    }
    catch(std::exception& ex)
    {
        std::cout << "错误信息:[" << ex.what() << "]" << std::endl;
    }
    
    return 0;
}