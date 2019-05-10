/****************************************************************
* 此文件由转译器自动生成， 请勿随意进行修改
*****************************************************************/ 
#ifndef _HEHE_HELLOWORLD_H_
#define _HEHE_HELLOWORLD_H_
#include <algorithm>
#include <hrpc_serializeStream.h>
#include <hrpc/HandleBase.h>
#include <hrpc/HrpcProtocol.h>
#include <hrpc/ObjectProxy.h>
using namespace Hrpc;
namespace Hehe
{

class HelloWorldProxy : public ObjectProxy
{
public:
Hrpc::Int32 test_hello(const std::string& name, std::string& outName) 
{
	Hrpc_SerializeStream s;
	Hrpc_Buffer buf(512, 32);
	Hrpc::Int32 ret;
	s.setBuffer(std::move(buf));
	s.write(1, name);
	s.write(2, outName);
	auto response = involve(ObjectProxy::HRPC_FUNC, "test_hello", std::move(s.getBuffer()));
	s.setBuffer(std::move(response));
	s.read(0, ret);
	s.read(2, outName);
	return ret;
}

};



class HelloWorld : public HandleBase
{
public:
~HelloWorld() {}
std::string getObjectName() const override
{	return "HelloWorld";
}

virtual Hrpc::Int32 test_hello(const std::string& name, std::string& outName) 
{
	throw Hrpc_Exception("[HelloWorld]: not implement the test_hello function");
}


Hrpc_Buffer distribute(const std::string& funcName, Hrpc_Buffer&& msg) override
{
	static std::string __funcContainer[] = {"test_hello"};
	auto itr = std::find(std::begin(__funcContainer), std::end(__funcContainer), funcName);
	if (itr != std::end(__funcContainer))
	{
		auto diff = itr - std::begin(__funcContainer);
		Hrpc_SerializeStream s;
		s.setBuffer(std::move(msg));
		switch (diff)
		{
			case 0:
			{
				Hrpc::Int32 ret;
				std::string name;
				s.read(1, name);
				std::string outName;
				s.read(2, outName);
				ret = test_hello(name, outName);
				Hrpc_Buffer response;
				s.setBuffer(std::move(response));
				s.write(0, ret);
				s.write(2, outName);
				return std::move(s.getBuffer());
			}
			default:
			{
				throw Hrpc_Exception(" unknown funcName = [" + funcName + "]");
			}
		};
	}
	else
	{
		throw Hrpc_Exception("[HelloWorld::distribute]: unknow funcName");
	}
	return Hrpc_Buffer();
}
};

}
#endif
