#include <hrpc_shm.h>

namespace Hrpc
{

void* Hrpc_Shm::create(::key_t key, std::size_t size)
{
    if (_shm_id >= 0)
    {
        throw Hrpc_ShmException("shm has been created");
    }
    // 仅创建
    int id = ::shmget(key, size, IPC_CREAT|IPC_EXCL|0666);    
    if (id < 0)
    {
        throw Hrpc_ShmException("create shm error, errno saved", errno);
    }

    // 链接到 当前进程空间
    void* addr = ::shmat(id, 0, 0);
    if (addr == (void*)-1)
    {
        throw Hrpc_ShmException("at shm error, errno saved", errno);   
    }

    // 赋值
    _shm_id = id;
    _addr = addr;
    return addr;
}

void* Hrpc_Shm::connect(::key_t key)
{
    if (_shm_id >= 0)
    {
        throw Hrpc_ShmException("shm has been created");
    }
    int id = ::shmget(key, 0, 0);    

    if (id < 0)
    {
        throw Hrpc_ShmException("connect shm error, errno saved", errno);
    }

    // 链接到 当前进程空间
    void* addr = ::shmat(id, 0, 0);
    if (addr == (void*)-1)
    {
        throw Hrpc_ShmException("at shm error, errno saved", errno);   
    }

    // 赋值
    _shm_id = id;
    _addr = addr;
    return addr;
    
}
void* Hrpc_Shm::getShmAddr() const
{
    if (_shm_id < 0)
    {
        throw Hrpc_ShmException("shm not created or connected");
    }

    return _addr;
}

int Hrpc_Shm::getShmId() const
{
    if (_shm_id < 0)
    {
        throw Hrpc_ShmException("shm not created or connected");
    }

    return _shm_id;
}

std::size_t Hrpc_Shm::getShmSize()
{
    if (_shm_id < 0)
    {
        throw Hrpc_ShmException("shm not created or connected");
    }

    // 获取参数
    ::shmid_ds data;
    int res = ::shmctl(_shm_id, IPC_STAT, &data);
    if (res < 0)
    {
        throw Hrpc_ShmException("shmctl error, errno saved", errno);
    }

    return data.shm_segsz;
}

}