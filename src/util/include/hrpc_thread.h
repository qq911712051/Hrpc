/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 线程相关操作
 * @Date: 2019-03-25 13:41:10
 */

#ifndef HRPC_THREAD_H_
#define HRPC_THREAD_H_
#include <pthread.h>
#include <hrpc_exception.h>
#include <hrpc_lock.h>
namespace Hrpc
{ 
/**
 * @description: 线程异常类
 */
class Hrpc_ThreadException : public Hrpc_Exception
{
public:
    Hrpc_ThreadException(const std::string& str) : Hrpc_Exception(str) {}
    Hrpc_ThreadException(const std::string& str, int err) : Hrpc_Exception(str, err) {}
    ~Hrpc_ThreadException() {}
};

/**
 * @description: 线程基类， 封装了pthread线程的相关操作
 *      使用线程时， 只需要继承此类， 并重写run函数， 调用start进行启动
 */
class Hrpc_Thread
{

public:
    /**
     * @description: 构造线程类
     * @param {type} 
     * @return: 
     */
    Hrpc_Thread() : _running(false), _tid(0) {} 

    virtual ~Hrpc_Thread() = 0;

    /**
     * @description: 新建线程并执行 
     * @return: 
     */
    void start();

    /**
     * @description: 线程实现。
     *              具体由派生类实现
     * @param {type} 
     * @return: 
     */
    virtual void run() = 0;    

    /**
     * @description: 对于joinable线程， 等待线程结束, 并回收线程资源
     * @param {type} 
     * @return: 
     */
    void join();

    /**
     * @description: 分离线程， 若线程结束运行，则系统直接回收资源
     * @param {type} 
     * @return: 
     */
    void detach();
    
    /**
     * @description: 获取线程id
     * @param {type} 
     * @return: 
     */
    pthread_t get_id() const {return _tid;}

    /**
     * @description: 判断线程是否正在执行
     * @param {type} 
     * @return: 
     */
    bool isRunning() const {return _running;}

private:
    /**
     * @description: 线程的入口函数， 线程执行从这里开始
     *             期间调用派生类的run函数
     * @param {type} 
     * @return: 
     */
    static void* enterFunc(void*);
private:

    bool _running; // 是否正在运行
    pthread_t _tid; // 线程id
    Hrpc_ThreadLock _lock;  // 保证start函数结束前， 新线程已经开始运行

};

}
#endif