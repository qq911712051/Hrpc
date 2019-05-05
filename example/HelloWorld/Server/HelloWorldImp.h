#ifndef HELLO_WORLD_IMP_H_
#define HELLO_WORLD_IMP_H_
#include <iostream>

#include <hrpc_common.h>

#include <HelloWorld.h>
class HelloWorldImp : public HelloWorld
{

public:
    Hrpc::Int32 test_hello(const std::string& name, std::string& outName) override
    {
        std::cout << "get a request test_hello" << std::endl;
        
        outName = name + Hrpc::Hrpc_Common::tostr(name.size());
        return name.size();
    }
};
#endif