/*
 * @author: blackstar0412
 * @github: github.com/qq911712051
 * @description: 这是一个用来接收数据的缓冲区
 * @Date: 2019-04-23 17:33:34
 */
#ifndef CONNECTION_BUFFER_H_
#define CONNECTION_BUFFER_H_

#include <vector>

#include <hrpc_buffer.h>
namespace Hrpc
{

/**
 * @description: 一个缓冲区buffer， 用来存储信息
 */
class ConnectionBuffer
{
public:
    
public:
    ConnectionBuffer() = default;

    /**
     * @description: 向缓冲区中压入数据, 只接收右值
     * @param: buffer 单个缓冲区
     * @return: 
     */
    void pushData(Hrpc_Buffer&& buffer);

    /**
     * @description: 获取当前缓冲区中数据的大小 
     * @param:
     * @return: 返回数据大小 
     */
    size_t size();

    /**
     * @description: 获取buffer中内容
     * @param: pos 数据的位置
     * @param: n 数据长度
     * @return: 返回一个Hrpc_Buffer缓冲区 
     */
    Hrpc_Buffer get(size_t pos, size_t n);
    
    /**
     * @description: 获取buffer中内容
     * @param: pos 数据的位置
     * @param: n 数据长度
     * @return: 返回一个std::string 字符串
     */
    std::string getString(size_t pos, size_t n);

    /**
     * @description: 移动数据游标
     * @param {type} 
     * @return: 
     */
    bool skip(size_t n);
    
private:
    
private:
    /**
     * @description: 去掉不必要的构造函数以及赋值函数 
     * @param {type} 
     * @return: 
     */
    ConnectionBuffer(const ConnectionBuffer&) = delete;
    ConnectionBuffer& operator=(const ConnectionBuffer&) = delete;
    ConnectionBuffer(ConnectionBuffer&&) = delete;
    ConnectionBuffer& operator=(ConnectionBuffer&&) = delete;
private:
    std::vector<Hrpc_Buffer> _buf_vec;      // 缓冲区数组
    int _index = {0}; // 数据起始点所在buffer的下标
    int _cur = {0};   // 数据起始点在buffer中的位置
    
};

}

#endif