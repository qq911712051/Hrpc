/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 
 * @Date: 2019-04-26 14:35:00
 */
#ifndef HANDLE_THREAD_H_
#define HANDLE_THREAD_H_

#include <memory>
#include <map>

#include <hrpc_thread.h>
#include <hrpc_baseProtocol.h>
#include <hrpc_exception.h>

#include <HrpcProtocol.h>
#include <HeartProtocol.h>
#include <common.h>

namespace Hrpc
{

class BindAdapter;

class HandleThread : public Hrpc_Thread
{
public:
    using Protocol = std::unique_ptr<Hrpc_BaseProtocol>;
private:
    using ProtocolFactory = std::map<std::string, Protocol>;
public:
    
    /**
     * @description: 构造函数
     * @param: bind 端口对象
     * @return: 
     */
    HandleThread(BindAdapter* bind);

    /**
     * @description: 析构函数,等待线程结束
     * @param {type} 
     * @return: 
     */
    ~HandleThread();

    /**
     * @description: 进行一些初始化操作 
     * @param {type} 
     * @return: 
     */
    void intialize();

    /**
     * @description: 线程执行的函数
     * @param {type} 
     * @return: 
     */
    void run() override;

    /**
     * @description: 添加业务线程的解析协议
     * @param: proto 协议类型 
     * @param: protoName 协议名称 
     * @return: 
     */
    void addHandleProtocol(Protocol&& proto, const std::string protoName);


    /**
     * @description: 设置心跳协议
     * @param {type} 
     * @return: 
     */
    void setHeartProtocol(Protocol&& proto);

    /**
     * @description: 结束此业务线程 
     * @param: 
     * @return: 
     */
    void terminate() {_terminate = true;}

private:

    /**
     * @description: 处理网络线程发出的心跳请求 
     * @param: req 请求
     * 
     * @return: 
     */
    void processHeart(const RequestPtr& req);

    /**
     * @description: 处理正常的协议
     * @param {type} 
     * @return: 
     */
    void processFunc(const RequestPtr& req);

    /**
     * @description: 检查buf是否格式正常
     * @param: buf, 传入的请求包
     * @param: protoName 解析出的协议名称
     * @return: 
     */
    bool checkRequest(const Hrpc_Buffer& buf, std::string& protoName);

    /**
     * @description: 对请求包内容按照协议进行分发
     * @param: buf  请求包的实际内容
     * @param: name 网络协议名称
     * @return: 
     */
    Hrpc_Buffer distribute(Hrpc_Buffer&& buf, const std::string& name);
public:
    BindAdapter*        _bindAdapter;       // 业务线程所属于的监听端口

    ProtocolFactory     _protoFactory;      // 业务处理协议工厂

    bool                _terminate = {false};       // 停止
};
}
#endif