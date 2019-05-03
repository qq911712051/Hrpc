#ifndef HRPC_CLIENT_H_
#define HRPC_CLIENT_H_

#include <memory>
#include <map>
#include <mutex>

#include <hrpc_config.h>

#include <ClientNetThreadGroup.h>
#include <ObjectProxy.h>

namespace Hrpc
{

/**
 *  运行Hrpc协议的客户端， 进行函数请求， 运行Hrpc协议
 */
class HrpcClient
{
    using ThreadGroupPtr = std::unique_ptr<ClientNetThreadGroup>;
    using ProxyPtr = std::unique_ptr<ObjectProxy>;
    using ObjectFactory = std::map<std::string, ProxyPtr>;
public:
    /**
     * @description: 构造函数， 构造hrpc客户端
     * @param: configFile   配置文件 
     * @return: 
     */
    HrpcClient(const std::string& configFile = "");

    /**
     * @description: 载入配置文件
     * @param {type} 
     * @return: 
     */
    void loadFile(const std::string& configFile);

    /**
     * @description: 通过对象名 获取 代理对象
     * 
     *  使用方法： 如果配置文件已经配置 objectName对应的ip和port， 那么直接使用objectName即可
     *            若配置文件没有配置， 必须在objectName中指明
     *          例如， object为Hehe
     *      那么objectName为 "Hehe@127.0.0.1:8888" 这种格式
     * @param: objectName 对象名称
     * @return: 
     */
    template<typename ObjectT>
    ObjectT* stringToProxy(const std::string& objectName);

    /**
     * @description: 关闭Hrpc客户端, 包括所有网络线程 
     * @param {type} 
     * @return: 
     */
    void terminate();
private:
    /**
     * @description: 初始化整个hrpc客户端
     * @param {type} 
     * @return: 
     */
    void intialize();

    /**
     * @description: 分析ObjectName是否携带地址信息
     * @param: objectName   对象名称
     * @param: ip           远端对象的ip
     * @param: port         远端对象的端口
     * @return: 
     *          返回是否携带地址信息
     */
    bool check(const std::string& objectName, std::string& object, std::string& ip, short& port);
private:
    ThreadGroupPtr      _netGroup;  // 指向网络线程组

    Hrpc_Config         _config;    // 配置文件

    ObjectFactory       _objectFactory; // 代理对象工厂
    std::mutex          _lock;          // 保护代理工厂的线程安全

    std::once_flag      _onceFlag;     // 初始化flag
    bool                _isInitialize = {false};  // 是否初始化
};

template<typename ObjectT>
ObjectT* HrpcClient::stringToProxy(const std::string& objectName)
{
    // 初始化
    std::call_once(_onceFlag, &HrpcClient::intialize, this);

    // 是否存在
    {
        std::lock_guard<std::mutex> sync(_lock);
        auto itr = _objectFactory.find(objectName);
        if (itr != _objectFactory.end())
        {
            // 将其强制转化为 派生类对象
            return (ObjectT*)itr->second.get();
        }
    }

    // 分析objectName组成
    std::string ip;
    std::string object;
    short port = 0;
    bool has = check(objectName, object, ip, port);
    if (has)
    {
        if (ip == "" || port <= 0)
        {
            throw Hrpc_Exception("[HrpcClient::stringToProxy]: objectName format error, objectName = [" + objectName + "]");
        }
        // 生成新的proxy
        auto proxy = ProxyPtr(new ObjectProxy(_netGroup.get(), object, ip, port));
        auto oPtr = proxy.get();
        {
            std::lock_guard<std::mutex> sync(_lock);
            auto itr = _objectFactory.find(objectName);
            if (itr != _objectFactory.end())
            {
                // 将其强制转化为 派生类对象
                return (ObjectT*)itr->second.get();
            }
            _objectFactory[objectName] = std::move(proxy);
        }
        return (ObjectT*)oPtr;
    }
    else
    {
        // 查询已经存在的远端端口以及ip
        auto& node = _config.getConfigNode("/hrpc/client/ObjectProxy");
        // 遍历所有的proxy， 查看是否存在相关object的信息
        for (auto& x : node->_map)
        {
            // 找到相关的object
            if (x.second->_option == objectName)
            {
                auto& obj = x.second->_map;

                ip = obj["IP"]->_value;
                port = Hrpc_Common::strto<short>(obj["Port"]->_value);
                
                if (ip == "" || port <= 0)
                {
                    throw Hrpc_Exception("[HrpcClient::stringToProxy]: objectProxy[" + objectName + "] config error");
                }

                auto proxy = ProxyPtr(new ObjectProxy(_netGroup.get(), objectName, ip, port));
                auto oPtr = proxy.get();
                {
                    std::lock_guard<std::mutex> sync(_lock);
                    auto itr = _objectFactory.find(objectName);
                    if (itr != _objectFactory.end())
                    {
                        // 将其强制转化为 派生类对象
                        return (ObjectT*)itr->second.get();
                    }
                    _objectFactory[objectName] = std::move(proxy);    
                }
                return (ObjectT*)oPtr;
            }
        }
    }
    
    return nullptr;
}

}
#endif