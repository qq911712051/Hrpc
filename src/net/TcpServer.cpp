#include <thread>
#include <iostream>
#include <chrono>

#include <hrpc_common.h>

#include <TcpServer.h>
using namespace std::chrono;

namespace Hrpc
{
void TcpServer::exec()
{
    if (!_config.isValid())
    {
        throw Hrpc_TcpServerException("[TcpServer::exec]: not load config-file");
    }
    init();         // 初始化TcpServer
    std::cout << "[TcpServer::exec]: intialize the TcpServer succ" << std::endl;

    intialize();    // 调用子类的初始化函数
    std::cout << "[TcpServer::exec]: call user-defined intializer succ" << std::endl;
    // 网络线程启动
    _net->start();
    std::cout << "[TcpServer::exec]: NetThread start!" << std::endl;
    // 业务线程启动
    for (auto& x : _bindAdapterFactory)
    {
        x.second->start();
    }
    std::cout << "[TcpServer::exec]: handleThread start!" << std::endl;
    
    // 判断系统的运行状态
    while (checkStatus())
    {
        
        std::this_thread::sleep_for(seconds(1));
    }


    // 执行用户自定义的销毁程序
    destroy();
}

void TcpServer::init()
{
    // 首先初始化网络线程组
    _net = NetGroupPtr(new NetThreadGroup());   
    _net->initialize(_config);

    // 初始化BindAdapter组
    std::cout << "search BindAdapter..." << std::endl;
    std::cout << "------------------------" << std::endl;
    auto& node = _config.getConfigNode("/hrpc/server/BindAdapter");

    // 配置文件中没有添加任何BindAdapter
    if (node->_map.size() == 0)
    {
        throw Hrpc_TcpServerException("[TcpServer::init]: not found BindAdapter");
    }
    for (auto& bind : node->_map)
    {
        std::cout << "-------" << bind.first << "--------" << std::endl; 
        for (auto& item : bind.second->_map)
        {
            std::cout << item.first << ":" << item.second->_value << std::endl;
        }
        std::cout << "------------------------" << std::endl;

        // 读取IP
        std::string host = bind.second->_map["IP"]->_value;
        if (host == "")
        {
            throw Hrpc_TcpServerException("[TcpServer::init]: BindAdapter[" + bind.first + "] not found 'IP'"); 
        }
        // 读取监听端口
        std::string port = bind.second->_map["Port"]->_value;
        if (port == "")
        {
            throw Hrpc_TcpServerException("[TcpServer::init]: BindAdapter[" + bind.first + "] not found 'Port'"); 
        }
        std::int16_t sPort = Hrpc_Common::strto<std::int16_t>(port);
        
        if (sPort <= 0)
        {
            throw Hrpc_TcpServerException("[TcpServer::init]: BindAdapter[" + bind.first + "]  'Port' is error format"); 
        }

        // 读取对应的业务线程数量
        std::string threadNum = bind.second->_map["HandleThreadNum"]->_value;
        if (threadNum == "")
        {
            throw Hrpc_TcpServerException("[TcpServer::init]: BindAdapter[" + bind.first + "] not found 'HandleThreadNum'"); 
        }
        
        int num = Hrpc_Common::strto<int>(threadNum);
        if (num < 0)
            num = 1;
        else if (num > 128)
            num = 16;
        
        // 创建BindAdapter
        auto pBind = new BindAdapter(_net, host, sPort, num, bind.first);
        // 初始化bindadapter
        pBind->initialize();
        
        _bindAdapterFactory[bind.first] = pBind;

        // 添加bindapter到网络线程
        _net->addBindAdapter(pBind);
    }
}

void TcpServer::loadConfig(const std::string& file)
{
    _config.parse(file);
}

template<class Protocol>
void TcpServer::addHandleProtocol(const std::string& object)
{
    auto itr = _bindAdapterFactory.find(object);
    if (itr != _bindAdapterFactory.end())
    {
        itr->second->addProtocol<Protocol>();
    }
    else
    {
        throw Hrpc_TcpServerException("[TcpServer::addHandleProtocol]: not found BindAdapter [" + object + "]");
    }
    
}

template<class Protocol>
void TcpServer::setHeartProtocol(const std::string& object)
{
    auto itr = _bindAdapterFactory.find(object);
    if (itr != _bindAdapterFactory.end())
    {
        itr->second->setHeartProtocol<Protocol>();
    }
    else
    {
        throw Hrpc_TcpServerException("[TcpServer::setHeartProtocol]: not found BindAdapter [" + object + "]");
    }
}

TcpServer::~TcpServer()
{
    // 释放资源
    for (auto& x : _bindAdapterFactory)
    {
        if (x.second)
        {
            delete x.second;
        }
    }
    // 释放网络线程组
    delete _net;
}

bool TcpServer::checkStatus()
{
    auto netR = _net->isRunning();
    if (!netR)
    {
        return false;
    }

    for (auto& x : _bindAdapterFactory)
    {
        if (!x.second->isRunning())
            return false;
    }
    return true;
}

}