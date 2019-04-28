#include <net/TcpServer.h>

#include <iostream>
using namespace Hrpc;

class MyApp : public TcpServer
{
public:
    void intialize() override
    {
        std::cout << "call user define intilize" << std::endl;
    }
    void destroy() override
    {
        std::cout << "call user define destroy" << std::endl;
    }
};
int main()
{
    MyApp server;
    server.loadConfig("/home/abel/study/coding/1.cfg");
    server.exec();

    return 0;
}