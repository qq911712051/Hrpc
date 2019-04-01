#include <iostream>
#include <cstring>

#include <hrpc_buffer.h>

namespace Hrpc
{


Hrpc_Buffer::Hrpc_Buffer(size_t len, size_t before)
{
    _buffer = 0;
    _before = before;
    try
    {
        if (len <= 256)
            len = 256;
        
        // 这里buffer前面预留
        _cap = len + _before;
        _cur = _before;
        _end = _before;

        _buffer = new char[_cap];
    } 
    catch (const std::exception& e)
    {
        std::cerr << "[Hrpc_Buffer::Hrpc_Buffer]: " << e.what() << std::endl;
        _buffer = 0;
    }
}

Hrpc_Buffer::~Hrpc_Buffer()
{
    if (_buffer)
    {
        delete [] _buffer;
    }
}

bool Hrpc_Buffer::write(const std::string& data)
{
    return write(data.c_str(), data.size());
}

bool Hrpc_Buffer::write(const char* data, int len)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::write]: _buffer is null");
    if (len < 0)
        len = ::strlen(data);


    size_t free = freeSize();
    if (len <= free)
    {
        // 直接复制数据到缓冲区
        ::memcpy(_buffer + _end, data, len);
        
        // 更新数据
        _end += len;
    }
    else
    {
        // 需要扩容
        size_t expand = (size() + len) * 2 + _before;
        bool res = expandBuffer(expand);
        if (!res)
            return false;

        // 将新插入数据复制到新的缓冲区
        ::memcpy(_buffer + _end, data, len);

        // 更新游标
        _end += len;
    }
        
    return true;
}

bool Hrpc_Buffer::expandBuffer(size_t len)
{
    try
    {
        char* tmp = new char[len];
        size_t buffer_size = _end - _cur;
        if (buffer_size > len)
            throw std::runtime_error("param error");
        // 复制数据到新缓冲区
        ::memcpy(tmp + _before, _buffer + _cur, buffer_size);
        // 更新游标
        _cur = _before;
        _end = _cur + buffer_size;

        // 释放旧缓冲区
        delete [] _buffer;
        _buffer = tmp;
    }
    catch (std::exception& e)
    {
        std::cerr << "[Hrpc_Buffer::expandBuffer]: " << e.what() << ", now need buffer size = " << len << std::endl;
        return false;
    }
    return true;
}

bool Hrpc_Buffer::skipBytes(size_t n)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::skipBytes]: _buffer is null");
    if (_end - _cur >= n)
        _cur += n;
    else
    {
        return false;
    }
    return true;
    
}


size_t Hrpc_Buffer::read(char* buffer, size_t len)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::read]: _buffer is null");
    if (len >= size())
    {
        ::memcpy(buffer, _buffer + _cur, size());
        return size();
    }
    else
    {
        ::memcpy(buffer, _buffer + _cur, len);
        return len;
    }
}

size_t Hrpc_Buffer::read(std::string& buffer)
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::read]: _buffer is null");
    buffer.resize(size());

    size_t has = read(&buffer[0], size());
    if (has != size())
    {
        throw Hrpc_BufferException("[Hrpc_Buffer::read]: read size error");
    }
    else
        return size();
}

void Hrpc_Buffer::clear()
{
    if (!_buffer)
        throw Hrpc_BufferException("[Hrpc_Buffer::clear]: _buffer is null");
    _cur = _before;
    _end = _before;
}
}
