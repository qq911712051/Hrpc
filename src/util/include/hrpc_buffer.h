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
#include <memory>

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
     * @description: 移动构造函数
     * @param {type} 
     * @return: 
     */
    Hrpc_Buffer(Hrpc_Buffer&& buffer);

    /**
     * @description: 移动赋值 函数
     * @param {type} 
     * @return: 
     */
    Hrpc_Buffer& operator=(Hrpc_Buffer&&);

    /**
     * @description: 释放资源 
     * @param {type} 
     * @return: 
     */
    ~Hrpc_Buffer();


    /**
     * @description: 以网络字节序 压入一个int值
     * @param {type} 
     * @return: 
     */
    bool appendFrontInt32(int data);
    bool appendInt32(int data);


    bool appendFrontInt8(std::int8_t data);
    bool appendInt8(std::int8_t data);

    bool appendFrontInt16(std::int16_t data);
    bool appendInt16(std::int16_t data);

    

    /**
     * @description: 查看数据前面4个字节的内容， 并将其从网络序转换为int32 
     * @param {type} 
     * @return: 
     */
    std::int32_t peekFrontInt32();
    std::int16_t peekFrontInt16();
    std::int8_t peekFrontInt8();
    
    
    
    /**
     * @description: 清空缓冲区所有内容
     * @param {type} 
     * @return: 
     */
    void clear();

    /**
     * @description: 将buffer中数据转化为字节码输出
     * @param {type} 
     * @return: 
     */
    std::string toByteString();

    /**
     * @description: 在当前buffer中寻找字符串data 
     * @param: data  目标字符串
     * @return: 如果找到， 返回其位置，否则返回-1
     */
    const char* find(const std::string& data) const;
    
    /**
     * @description: 获取指定地点的数据
     * @param: pos  数据位置
     * @param: len  数据长度
     * @return: 返回包含数据的字符串
     */
    std::string get(size_t pos, size_t len);

    /**
     * @description: 获取指定地点的数据
     * @param: pos  数据位置
     * @param: len  数据长度
     * @return: 返回一个包含数据的buffer 
     */
    Hrpc_Buffer getToBuffer(size_t pos, size_t len);

    /**
     * @description: 将当前位置游标移动到目标位置
     * @param: pos 目标位置
     * @return: 是否成功
     */
    bool skipTo(const char* pos);
    /**
     * @description: 写入数据
     * @param {type} 
     * @return: 
     */
    bool write(const std::string& data);
    
    bool write(const char* data, int len = -1);

    /**
     * @description: 压入数据到当前缓冲区
     * @param {type} 
     * @return: 
     */
    void pushData(Hrpc_Buffer&& buf);

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
     * @description: 禁止拷贝以及赋值
     * @param {type} 
     * @return: 
     */
    Hrpc_Buffer(const Hrpc_Buffer&) = delete;
    Hrpc_Buffer& operator=(const Hrpc_Buffer&) = delete;


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
typedef std::shared_ptr<Hrpc_Buffer>  Hrpc_BufferPtr;

}
#endif
