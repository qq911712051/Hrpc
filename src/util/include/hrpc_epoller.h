/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 
 * @Date: 2019-03-22 16:13:37
 */
#ifndef HRPC_EPOLLER_H_   
#define HRPC_EPOLLER_H_

#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
/**
 * @description: 封装epoll的相关操作 
 */

namespace Hrpc
{
class Hrpc_Epoller
{
public:
    /**
     * @description： 构造函数，初始化某些参数
     * @param:  et 是否边缘触发
     * @return: 
     */
    Hrpc_Epoller(bool et = true);

    /**
     * @description: 释放epoll fd以及相关资源
     * @param {type} 
     * @return: 
     */
    ~Hrpc_Epoller();

    /**
     * @description: 阻塞等待 
     * @param: millseconds 等待事件，单位ms
     *         参数若为-1， 表示阻塞，直到通知到来。
     *         参数为0， 表示非阻塞， 没有事件直接返回
     *         参数大于0， 表示等待的时间, 超时后直接返回0  
     * 
     * @return:  返回值大于0， 表示新通知的数量
     *           返回值等于0， 表示超时
     *           返回值小于0， 表示发生错误，保存在errno中
     */
    int wait(uint32_t millseconds = 10);

    /**
     * @description: 创建epoll并且初始化
     * @param: conn_num 最大连接数 
     *         新内核忽略这个参数
     * @return: 是否成功创建
     */
    bool createEpoll(size_t conn_num);

    /**
     * @description: 添加描述符到epoll中
     * @param {type} 
     * @return: 
     */
    void add(int fd, uint64_t data, uint32_t event);

    /**
     * @description: 修改监听的事件
     * @param {type} 
     * @return: 
     */
    void mod(int fd, uint64_t data, uint32_t event);

    /**
     * @description: 从epoll中删除描述符
     * @param {type} 
     * @return: 
     */
    void del(int fd, uint64_t data = 0, uint32_t event = 0);
private:

    /**
     * @description:  禁止此对象进行拷贝以及赋值
     */
    Hrpc_Epoller(const Hrpc_Epoller&);
    Hrpc_Epoller& operator=(const Hrpc_Epoller&);
    
    /**
     * @description: 调动epoll_ctrl进行具体的操作
     * @param {type} 
     * @return: 
     */
    void ctrl(int op, int fd, epoll_event* event);
private:
    bool _bEt; // 是否边缘触发
    int _epoll_fd;   // epoll描述符
    size_t _max_connection; // 最大的连接数

    epoll_event* _pEvents;  // 事件数组
};
}
#endif
