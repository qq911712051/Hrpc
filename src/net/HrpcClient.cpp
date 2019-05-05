#include <iostream>

#include <hrpc_common.h>


#include <ClientNetThreadGroup.h>
#include <HrpcClient.h>
namespace Hrpc
{

HrpcClient::HrpcClient(const std::string& configFile)
{
    if (configFile == "")
    {
        // 默认情况， 没有载入任何配置文件
    }
    else
    {
        try
        {
            _config.parse(configFile);
        }
        catch(const std::exception& e)
        {
            std::cerr << "[HrpcClient::HrpcClient]:" << e.what() << std::endl;
        }
    }
    
}

void HrpcClient::intialize()
{
    if (!_config.isValid())
    {
        throw Hrpc_Exception("[HrpcClient::intialize]: not found config file");
    }
    // 初始化网络线程组等等
    _netGroup = ThreadGroupPtr(new ClientNetThreadGroup);
    _netGroup->intialize(_config);
    _netGroup->start();
    
    // 读取waitTime
    size_t waitTime = Hrpc_Common::strto<size_t>(_config.getString("/hrpc/client/HrpcWaitTime"));
    if (waitTime > 0)
        _waitTime = waitTime;
    _isInitialize = true;
}

void HrpcClient::loadFile(const std::string& configFile)
{
    if (!_config.isValid())
    {
        _config.parse(configFile);
    }
    else
    {
        std::cerr << "[HrpcClient::loadFile]: load config error, the hrpc-client has intialized by other config file" << std::endl;
    }
    
}

bool HrpcClient::check(const std::string& objectName, std::string& object, std::string& ip, short& port)
{
    auto pos = objectName.find("@");
    if (pos != std::string::npos)
    {
        object = std::string(objectName.begin(), objectName.begin() + pos); // 拆出 object名称
        auto pos2 = objectName.find(":");
        ip = std::string(objectName.begin() + pos + 1, objectName.begin() + pos2);
        port = Hrpc_Common::strto<short>(std::string(objectName.begin() + pos2 + 1, objectName.end()));
    }
    object = objectName;
    return false;
}

void HrpcClient::terminate()
{
    _netGroup->terminate();
}

ObjectProxy* HrpcClient::getObjectProxy(const std::string& objectName)
{
    // 初始化
    std::call_once(_onceFlag, &HrpcClient::intialize, this);

    // 是否存在
    {
        std::lock_guard<std::mutex> sync(_lock);
        auto itr = _objectFactory.find(objectName);
        if (itr != _objectFactory.end())
        {
            return itr->second.get();
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
            throw Hrpc_Exception("[HrpcClient::getObjectProxy]: objectName format error, objectName = [" + objectName + "]");
        }
        // 生成新的proxy
        auto proxy = ProxyPtr(new ObjectProxy(_netGroup, object, ip, port, _waitTime));
        auto oPtr = proxy.get();
        {
            std::lock_guard<std::mutex> sync(_lock);
            auto itr = _objectFactory.find(objectName);
            if (itr != _objectFactory.end())
            {
                return itr->second.get();
            }
            _objectFactory[objectName] = std::move(proxy);
        }
        return oPtr;
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
                    throw Hrpc_Exception("[HrpcClient::getObjectProxy]: objectProxy[" + objectName + "] config error");
                }

                auto proxy = ProxyPtr(new ObjectProxy(_netGroup, objectName, ip, port, _waitTime));
                auto oPtr = proxy.get();
                {
                    std::lock_guard<std::mutex> sync(_lock);
                    auto itr = _objectFactory.find(objectName);
                    if (itr != _objectFactory.end())
                    {
                        return itr->second.get();
                    }
                    _objectFactory[objectName] = std::move(proxy);    
                }
                return oPtr;
            }
        }
    }
    
    return nullptr;
}

HrpcClient::~HrpcClient()
{
    if (_netGroup)
    {
        delete _netGroup;
    }
}
}