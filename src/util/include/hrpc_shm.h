#ifndef HRPC_SHM_H_
#define HRPC_SHM_H_

#include <sys/ipc.h>
#include <sys/shm.h>

#include <hrpc_exception.h>

namespace Hrpc
{

/**
 * @description: 共享内存操作出现的异常
 * @param {type} 
 * @return: 
 */
class Hrpc_ShmException : public Hrpc_Exception
{
public:
    Hrpc_ShmException(const std::string& str) : Hrpc_Exception(str) {}
    Hrpc_ShmException(const std::string& str, int errCode) : Hrpc_Exception(str, errCode) {}
    ~Hrpc_ShmException() {}
}; 
 

/**
 * 对于共享内存操作的一个封装
 */
class Hrpc_Shm
{
public:
    Hrpc_Shm() = default;

    /**
     * @description: 新建一片共享内存， 如果失败， 抛异常
     * @param {type} 
     * @return: 
     */
    void* create(::key_t key, std::size_t size);

    /**
     * @description: 链接已存在的共享内存
     * @param {type} 
     * @return: 
     */
    void* connect(::key_t key);

    /**
     * @description: 获取共享内存位置
     * @param {type} 
     * @return: 
     */
    void* getShmAddr() const;


    /**
     * @description: 获取shmid 
     * @param {type} 
     * @return: 
     */
    int getShmId() const;

    /**
     * @description: 获取共享内存大小, 单位bytes
     * @param {type} 
     * @return: 
     */
    std::size_t getShmSize();
public:
    int         _shm_id = {-1};    // 共享内存id
    void*       _addr = {nullptr}; // 共享内存的位置      
};
}
#endif