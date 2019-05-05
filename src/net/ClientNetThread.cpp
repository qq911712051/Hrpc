#include <thread>

#include <hrpc_common.h>

#include <ClientNetThread.h>

using namespace std::placeholders;
namespace Hrpc
{

ClientNetThread::ClientNetThread(ClientNetThreadGroup* group)
{
    _threadGroup = group;
}

void ClientNetThread::intialize(const Hrpc_Config& config)
{
    // 设置回调函数
    _server.setCloseCallback(std::bind(&ClientNetThread::closeCallback, this, _1));
    _server.setRecvCallback(std::bind(&ClientNetThread::recvCallback, this, _1, _2));

    // 读取最大空闲时间
    int maxIdleTime = Hrpc_Common::strto<int>(config.getString("/hrpc/client/NetThread/MaxIdleTime"));
    if (maxIdleTime <= 0)
    {
        maxIdleTime = 10000;    // 默认10s
    }
    _maxIdleTime = maxIdleTime;

    // 读取epoll时间
    int epollTime = Hrpc_Common::strto<int>(config.getString("/hrpc/client/NetThread/EpollWaitTime"));
    if (epollTime <= 0)
    {
        epollTime = 10;
    }
    // 读取最大链接数
    int maxConn = Hrpc_Common::strto<int>(config.getString("/hrpc/client/NetThread/MaxConnection"));
    if (maxConn <= 0)
    {
        maxConn = 1024;
    }

    // 读取 每个链接最多的未响应请求数
    int maxWait = Hrpc_Common::strto<int>(config.getString("/hrpc/client/MaxWaitNumInConnection"));
    if (maxWait <= 0)
    {
        maxWait = 512;
    }
    _max_wait_num = maxWait;

    // 初始化epollerServer
    _server.init(maxConn, epollTime);

    // 添加定时任务
    _server.addTimerTaskRepeat(0, _maxIdleTime, &ClientNetThread::checkActivity, this);
}
void ClientNetThread::run()
{
    while (!_terminate)
    {
        _server.run();
    }
}


ClientConnectionPtr ClientNetThread::connect(const std::string& object, const std::string& ip, short port)
{
    {
        std::lock_guard<std::mutex> sync(_lock);
        auto itr = _list.find(object);
        if (itr != _list.end())
        {
            // 获得相关链接
            int uid = itr->second;

            auto ptr = _server.getConnectionByUid(uid);
            // 判断是否有效
            if (ptr)
            {
                // 返回ClientConnection
                return std::dynamic_pointer_cast<ClientConnection>(ptr);
            }
        }
    }
    // 链接不存在或者已经失效， 重新建立链接
    int fd = buildConnection(ip, port);
    if (fd >= 0)
    {
        auto ptr = ClientConnectionPtr(new ClientConnection(this, fd, _max_wait_num));
        // 添加到epoll中
        auto basePtr = std::static_pointer_cast<ConnectionBase>(ptr);
        addConnection(basePtr);
        
        // 等待同步完成, 这里可能会出现死循环， 当addConnection失败的时候， 因此加一个等待时间
        // 等待100ms
        if (!basePtr->waitForUidValid(100))
        {
            throw Hrpc_Exception("[ClientNetThread::connect]: connect error, maybe the connection is too much");
        }

        // 添加到链接有效链表
        {
            std::lock_guard<std::mutex> sync(_lock);
            auto itr = _list.find(object);
            if (itr != _list.end())
            {
                // 获得相关链接
                int uid = itr->second;

                auto newPtr = _server.getConnectionByUid(uid);
                // 判断是否有效
                if (newPtr)
                {
                    // 刚生成的链接为多余的链接,且没有使用。 可以直接关闭
                    closeConnection(basePtr->getUid());
                    // 返回ClientConnection
                    return std::dynamic_pointer_cast<ClientConnection>(newPtr);
                }
            }
            _list[object] = basePtr->getUid();
        }
        std::cout << "[ClientNetThread::connect]: build a connetion in NetThread[" << std::this_thread::get_id() << "], connection_id = " 
                    << basePtr->getUid() << std::endl;
        
        // 返回新的链接
        return ptr;
    }
    else
    {
        throw Hrpc_Exception("[ClientNetThread::connect]: connect error");
    }
    return ClientConnectionPtr();
}

int ClientNetThread::buildConnection(const std::string& ip, short port)
{
    Hrpc_Socket sock;
    sock.CreateSocket(SOCK_STREAM, AF_INET, 0);
    sock.setOwner(false);
    try
    {
        sock.connect(ip, port);
    }
    catch(const Hrpc_Exception& e)
    {
        std::cerr << "[ClientNetThread::buildConnection]:" << e.what() << ", ip = " << ip << ", port = " << port << std::endl;
        sock.close();
        return -1;
    }
    return sock.getFd();
}


void ClientNetThread::addConnection(const ConnectionPtr& ptr)
{
    auto task = [ptr, this]() {
        _server.addConnection(ptr);
    };
    auto resp = ResponsePtr(new ResponseMessage);
    resp->_type = ResponseMessage::HRPC_RESPONSE_TASK;
    resp->_task = std::unique_ptr<ResponseMessage::Task>(new ResponseMessage::Task(std::move(task)));

    _server.insertResponseQueue(std::move(resp));
}

void ClientNetThread::checkActivity()
{
    // 遍历一遍所有的connection， 寻找需要进行心跳检测的链接
    auto& _connections = _server.getConnections();
    for (auto&x : _connections)
    {
        auto nowTime = Hrpc_Time::getNowTimeMs();
        
        auto clientConn = std::dynamic_pointer_cast<ClientConnection>(x.second);

        auto diffTime = nowTime - clientConn->getLastSendTime(); 
        if (diffTime > _maxIdleTime)
        {
            std::cout << "[ClientNetThread::checkActivity]: NetThread[" << std::this_thread::get_id()
                    << "], conneciton-id:" << clientConn->getUid() << " idle-time over the limit" << std::endl;

            // 删除链接
            _server.closeConnection(clientConn->getUid());
            continue;
        }
    }
}

void ClientNetThread::recvCallback(EpollerServer* server, const ConnectionPtr& conn)
{
    // 有新数据到来
    auto res = conn->recvData();
    if (res == -1)
    {
        // 对端关闭
        _server.closeConnection(conn->getUid());
        std::cout << "[ClientNetThread::recvCallback]: NetThread[" << std::this_thread::get_id()
                            << "], conneciton-id:" << conn->getUid() << " recv 0 bytes data, close connection" << std::endl;
    }
    else if (res == -2)
    {
        _server.closeConnection(conn->getUid());
        std::cerr << "[ClientNetThread::recvCallback]: recv return -2 and errno = " << errno << std::endl;
    }

    // 检测收到包的 完整性
    bool checkComplete = false;
    do 
    {
        Hrpc_Buffer tmp;
        checkComplete = conn->extractPackage(tmp);
        if (tmp.size() != 0)
        {
            // 收到Hrpc应答包
            auto clientConn = std::dynamic_pointer_cast<ClientConnection>(conn);
            clientConn->parseHrpc(std::move(tmp));
        }

    } while(checkComplete);
}

void ClientNetThread::insertResponseQueue(ResponsePtr&& resp)
{
    _server.insertResponseQueue(std::move(resp));
}

void ClientNetThread::terminate()
{
    _terminate = true;
    _server.notify();
}

ClientNetThread::~ClientNetThread()
{
    terminate();
}

void ClientNetThread::closeConnection(int uid)
{
    _server.closeConnection(uid);
}

}