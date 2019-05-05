#ifndef HRPC_CLIENT_H_
#define HRPC_CLIENT_H_

#include <memory>
#include <map>
#include <mutex>

#include <hrpc_config.h>

#include <hrpc/ObjectProxy.h>
namespace Hrpc
{

class ClientNetThreadGroup;

/**
 *  运行Hrpc协议的客户端， 进行函数请求， 运行Hrpc协议
 */
class HrpcClient
{
    using ThreadGroupPtr = ClientNetThreadGroup*;
    using ProxyPtr = std::unique_ptr<ObjectProxy>;
    using ObjectFactory = std::map<std::string, ProxyPtr>;
public:
    /**
     * @description: 构造函数， 构造hrpc客户端
     * @param: configFile   配置文件 
     * @return: 
     */
    HrpcClient(const std::string& configFile = "");
    
    ~HrpcClient();

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
     * @description: 根据objectName获取objectProxy对象
     * @param {type} 
     * @return: 
     */
    ObjectProxy* getObjectProxy(const std::string& objectName);

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
    ThreadGroupPtr      _netGroup = {nullptr};  // 指向网络线程组

    Hrpc_Config         _config;    // 配置文件

    size_t              _waitTime = {2000};  // 默认的等待时间

    ObjectFactory       _objectFactory; // 代理对象工厂
    std::mutex          _lock;          // 保护代理工厂的线程安全

    std::once_flag      _onceFlag;     // 初始化flag
    bool                _isInitialize = {false};  // 是否初始化
};

template<typename ObjectT>
ObjectT* HrpcClient::stringToProxy(const std::string& objectName)
{
    auto ptr = getObjectProxy(objectName);
    if (ptr)
    {
        // 强制转化为派生类
        return (ObjectT*)ptr;
    }
    else
    {
        throw Hrpc_Exception("[HrpcClient::stringToProxy]: string to proxy error");
    }
}

}
#endif