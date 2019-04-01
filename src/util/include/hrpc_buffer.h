/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description:  缓冲区
 * @Date: 2019-03-31 14:40:38
 */
#ifndef HRPC_BUFFER_H_
#define HRPC_BUFFER_H_

#include <string>
#include <vector>

#include <hrpc_ptr.h>
#include <hrpc_exception.h>

namespace Hrpc
{

/**
 * buffer出现异常时抛出
 */
class Hrpc_BufferException : public Hrpc_Exception
{
public:
    Hrpc_BufferException(const std::string& str) : Hrpc_Exception(str) {}
    Hrpc_BufferException(const std::string& str, int err) : Hrpc_Exception(str, err) {}
    ~Hrpc_BufferException() {}
};
/**
 * @description: 自动扩容的数据缓冲区 , 非线程安全
 * @param {type} 
 * @return: 
 */
class Hrpc_Buffer
{
public:
    typedef size_t    size_type;
public:
    /**
     * @description: 初始化buffer参数
     * @param: size buffer初始时大小
     * @param: before   buffer前面预留的大小， 方便直接向前插入数据
     * @return: 
     */
    Hrpc_Buffer(size_t size = 1024, size_t before = 16);

    /**
     * @description: 释放资源 
     * @param {type} 
     * @return: 
     */
    ~Hrpc_Buffer();


    /**
     * @description: 清空缓冲区所有内容
     * @param {type} 
     * @return: 
     */
    void clear();

    
    /**
     * @description: 写入数据
     * @param {type} 
     * @return: 
     */
    bool write(const std::string& data);
    
    bool write(const char* data, int len = -1);

    /**
     * @description: 跳过n字节数据, 将位置游标后移
     * @param: n 跳过的字节数量
     * @return: 
     */
    bool skipBytes(size_t n);

    /**
     * @description: 读取数据
     * @param {type} 
     * @return: 
     */
    size_type read(char* buffer, size_t len);

    size_type read(std::string& buffer);

    /**
     * @description: 获取缓冲区数据大小
     * @param {type} 
     * @return: 
     */
    size_type  size() const {return _end - _cur;}

    /**
     * @description: 缓冲区头部地址
     * @param {type} 
     * @return: 
     */
    const char* begin() const {return _buffer + _cur;}

    /**
     * @description: 缓冲区尾部地址
     * @param {type} 
     * @return: 
     */
    const char* end() const {return _buffer + _end;}
private:
    /**
     * @description: 扩容buffer
     * @param: len 扩容后的buffer大小
     * @return: 
     */
    bool expandBuffer(size_t len);

    /**
     * @description: 获取缓冲区尾部剩余可用空间
     * @param {type} 
     * @return: 
     */
    size_t freeSize() const {return _cap - _end;}
private:
    char*                   _buffer;        // 缓冲区
    size_type               _cur;           // 当前游标位置
    size_type               _end;           // 结束游标位置
    size_t                  _cap;           // buffer容量
    size_t                  _before;        // buffer前端预留空间，便于直接在buffer前段插入数据
};
typedef Hrpc_SharedPtr<Hrpc_Buffer>  Hrpc_BufferPtr;

}
#endif
