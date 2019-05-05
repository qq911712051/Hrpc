#include <hrpc/TcpServer.h>
#include <HelloWorldImp.h>

#include <iostream>
#include <memory>


using namespace Hrpc;

class MyApp : public TcpServer
{
public:
    void intialize() override
    {
        std::unique_ptr<HrpcProtocol> proto(new HrpcProtocol);
        proto->setHandleObject<HelloWorldImp>();
        
        addHandleProtocol("HelloWorld", std::move(proto));

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