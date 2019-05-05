/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 服务器Server
 * @Date: 2019-03-31 20:22:33
 */

#ifndef HRPC_TCPSERVER_H_
#define HRPC_TCPSERVER_H_

#include <string>
#include <memory>
#include <map>
#include <vector>

#include <hrpc_exception.h>
#include <hrpc_config.h>
#include <hrpc_baseProtocol.h>


namespace Hrpc
{

class BindAdapter;
class NetThreadGroup;

class Hrpc_TcpServerException : public Hrpc_Exception
{
public:
    Hrpc_TcpServerException(const std::string& msg) : Hrpc_Exception(msg) {}
    Hrpc_TcpServerException(const std::string& msg, int err) : Hrpc_Exception(msg, err) {}
    ~Hrpc_TcpServerException() {}
};

/**
 *  服务端基类， 表示一个应用App
 */
class TcpServer
{
    using NetGroupPtr = NetThreadGroup*;
    using BindAdapterPtr = BindAdapter*;
    using BindAdapterFactory = std::map<std::string, BindAdapterPtr>;
public:
    /**
     * @description: 默认的构造函数
     * @param {type} 
     * @return: 
     */
    TcpServer() = default;

    /**
     * @description: 释放资源
     * @param {type} 
     * @return: 
     */
    ~TcpServer();

    /**
     * @description: 为相关的端口添加处理协议 
     * @param: object 端口对象名称
     * @return: 
     */
    void addHandleProtocol(const std::string& object, std::unique_ptr<Hrpc_BaseProtocol>&&);

    /**
     * @description: 为业务线程设置心跳协议 
     * @param {type} 
     * @return: 
     */
    void setHeartProtocol(std::unique_ptr<Hrpc_BaseProtocol>&&);

    /**
     * @description: 进行一些初始化操作， 申请一些资源等等
     * @param {type} 
     * @return: 
     */
    virtual void intialize() = 0;

    /**
     * @description: 进行一些收尾工作， 释放相关资源
     * @param {type} 
     * @return: 
     */
    virtual void destroy() = 0;

    /**
     * @description: 载入配置文件
     * @param: file 配置文件名称
     * @return: 
     */
    void loadConfig(const std::string& file);

    /**
     * @description: 开始执行 
     * @param {type} 
     * @return: 
     */
    void exec();
private:
    /**
     * @description: 根据config初始化整个tcpServer 
     * @param {type} 
     * @return: 
     */
    void init();

    /**
     * @description: 检查系统的运行状态, 状态异常返回false
     * @param {type} 
     * @return: 
     */
    bool checkStatus();
private:
    /**
     * @description: 禁止拷贝和赋值
     * @param {type} 
     * @return: 
     */
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;
private:
    NetGroupPtr     _net = {nullptr};       // 网络线程组
    
    BindAdapterFactory  _bindAdapterFactory;    // 端口对象工厂
    
    Hrpc_Config     _config;    // 配置文件

    bool            _terminate = {false}; // 停止运行
    
    std::vector<std::unique_ptr<Hrpc_BaseProtocol>> _protocolVec;   // 保存用户定义的协议
};

}
#endif