#ifndef HRPC_UID_H_
#define HRPC_UID_H_

#include <list>
#include <mutex>

namespace Hrpc
{
/**
 * 一个uid生成器
 */
class UidGenarator
{
public:
    /**
     * @description: 
     * @param: 
     * @return: 
     */
    UidGenarator() {}

    /**
     * @description: 初始化uid序列 
     * @param: maxConn 最大的uid 
     * @return: 
     */
    void init(int maxConn = 1024);

    /**
     * @description: 释放相关资源
     * @return: 
     */
    ~UidGenarator() {}

    /**
     * @description: 获取一个uid 
     * @param {type} 
     * @return: 返回值小于0时表示没有可用的uid
     */
    int popUid();

    /**
     * @description: 返还取出的uid 
     * @param {type} 
     * @return: 
     */
    void pushUid(int uid);
    
private:
    std::list<int>  _list;      // 可用的uid链表
    std::mutex      _lock;      
    size_t          _MaxUid = {0};
};
}
#endif