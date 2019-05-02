#include <iostream>

#include <hrpc_common.h>

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
    _netGroup = ThreadGroupPtr(new ClientNetThreadGroup(_config));
    _netGroup->intialize();
    _netGroup->start();
    
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
}