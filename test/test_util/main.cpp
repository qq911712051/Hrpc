#include <iostream>
#include <hrpc_ptr.h>
using namespace Hrpc;

class A
{
public:
    ~A()
    {
        std::cout << "A deconstruct" << std::endl;
    }
    int operator* () { return 11;}
};

void func(Hrpc_SharedPtr<A> ptr)
{

    std::cout << "addr = " << ptr.get() << std::endl;
    std::cout << "count = " << ptr.count() << std::endl;
}

int main()
{
    Hrpc_SharedPtr<A> ptr = new A();
    func(ptr);
    std::cout << "count = " << ptr.count() << std::endl;
    return 0;    
}