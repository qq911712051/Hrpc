/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description:  本文件所有代码均由转译器 自动生成
 * @Date: 2019-05-05 13:50:15
 */
#ifndef HELLO_WORLD_H_
#define HELLO_WORLD_H_
#include <algorithm>

#include <hrpc_serializeStream.h>

#include <hrpc/HandleBase.h>
#include <hrpc/HrpcProtocol.h>
#include <hrpc/ObjectProxy.h>

using namespace Hrpc;
/**
 * 
 * 普通的代理对象的实现
 */
class HelloWorldProxy : public ObjectProxy
{
public:

    // 具体实现的函数
    // 普通调用函数
    Hrpc::Int32 test_hello(const std::string& name, std::string& outName)
    {
        // 初始化环境
        Hrpc_SerializeStream s;
        Hrpc_Buffer buf(512, 32);
        Hrpc::Int32 ret = 0;
        s.setBuffer(std::move(buf));

        // 序列化参数
        s.write(1, name);
        s.write(2, outName);
        
        // 等待结果返回
        auto response = involve(ObjectProxy::HRPC_FUNC, "test_hello", std::move(s.getBuffer()));

        // 解析参数
        s.setBuffer(std::move(response));
        s.read(0, ret);
        s.read(2, outName);
        
        return ret;
    }
}; 


/**
 * 服务端处理代码
 */
class HelloWorld : public HandleBase
{
public:
    ~HelloWorld() {}

    std::string getObjectName() const override
    {
        return "HelloWorld";
    }
    
    virtual Hrpc::Int32 test_hello(const std::string& name, std::string& outName)
    {
        throw Hrpc_Exception("[HelloWorld]: not implement the test_hello function");
    }

    /**
     * @description: 这里进行函数的分发
     * @param {type} 
     * @return: 
     */
    Hrpc_Buffer distribute(const std::string& funcName, Hrpc_Buffer&& msg) override
    {
        static std::string __funcContainer[] = {"test_hello"};

        // 搜索
        auto itr = std::find(std::begin(__funcContainer), std::end(__funcContainer), funcName);
        
        if (itr != std::end(__funcContainer))
        {
            auto diff = itr - std::begin(__funcContainer);

            Hrpc_SerializeStream s;
            s.setBuffer(std::move(msg));
            // 选择
            switch (diff)
            {
                case 0:
                {
                    std::string name;
                    std::string outName;
                    Hrpc::Int32 ret = 0;

                    s.read(1, name);
                    s.read(2, outName);
                    ret = test_hello(name, outName);

                    // 封装返回
                    Hrpc_Buffer response;
                    s.setBuffer(std::move(response));
                    
                    s.write(0, ret);
                    s.write(2, outName);
                    return std::move(s.getBuffer());
                }
                default:
                    throw Hrpc_Exception("[HelloWorld::distribute]: unknown error");
            };
        }
        else
        {
            throw Hrpc_Exception("[HelloWorld::distribute]: unknow funcName");
        }
        
        return Hrpc_Buffer();
    }
};

#endif